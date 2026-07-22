class serial_reader : public commands_reader
{
public:
	void check_serial()
	{
		static byte rx_buffer[256];
		static int rx_size = 0;
		static int expected_size = 0;
		static unsigned long frame_start_ms = 0;
		static unsigned long last_byte_ms = 0;

		const unsigned long timeout_serial_rec = 100;
		const unsigned long full_cmd_timeout = 3000;
		unsigned long now = millis();

		if (rx_size > 0)
		{
			const bool timeout_between_bytes = (now - last_byte_ms) > timeout_serial_rec;
			const bool timeout_full_frame = (now - frame_start_ms) > full_cmd_timeout;
			if (timeout_between_bytes || timeout_full_frame)
			{
				rx_size = 0;
				expected_size = 0;
			}
		}

		while (Serial2.available())
		{
			byte current = Serial2.read();
			now = millis();

			if (rx_size == 0)
			{
				expected_size = ((int)current) + 1;

				// Minimum frame: [len][status][cmd][crc1][crc2]
				if (expected_size < 5 || expected_size > (int)sizeof(rx_buffer))
				{
					expected_size = 0;
					continue;
				}

				frame_start_ms = now;
			}

			rx_buffer[rx_size++] = current;
			last_byte_ms = now;

			if (expected_size > 0 && rx_size == expected_size)
			{
				if (is_valid_frame(rx_buffer, rx_size))
				{
					cmd_handler(bytes_to_hex_string(rx_buffer, rx_size));
					answer_rec = true;
				}

				// Invalid frame is silently discarded.
				rx_size = 0;
				expected_size = 0;
			}
		}
	}

private:
	bool is_valid_frame(const byte *frame, int frame_size)
	{
		if (frame == NULL || frame_size < 5)
			return false;

		const uint16_t crc = uiCrc16Cal(frame, frame_size - 2);
		const byte crc1 = crc & 0xFF;
		const byte crc2 = (crc >> 8) & 0xFF;

		return frame[frame_size - 2] == crc1 && frame[frame_size - 1] == crc2;
	}

	String bytes_to_hex_string(const byte *data, int size)
	{
		String result = "";
		result.reserve(size * 2);

		for (int i = 0; i < size; i++)
		{
			result += String((data[i] < 0x10) ? "0" : "") + String(data[i], HEX);
		}

		return result;
	}

	bool is_hex_string(const String &value)
	{
		if (value.length() == 0 || (value.length() % 2) != 0)
			return false;

		for (int i = 0; i < value.length(); i++)
		{
			const char c = value[i];
			const bool is_num = (c >= '0' && c <= '9');
			const bool is_lower_hex = (c >= 'a' && c <= 'f');
			const bool is_upper_hex = (c >= 'A' && c <= 'F');
			if (!is_num && !is_lower_hex && !is_upper_hex)
				return false;
		}

		return true;
	}

	void cmd_handler(String cmd)
	{
		if (cmd.length() < 10)
			return;

		if (debug_mode)
			myserial.write("[ DEBUG ] <- " + cmd);

		String status = cmd.substring(2, 4);
		String reader_cmd = cmd.substring(4, 6);

		if (status == "00")
		{
			// SETUP
			if (reader_cmd == "21")
			{
				if (cmd.substring(13, 14) == "1")
					one_ant = true;
				else
					one_ant = false;
				myserial.write("#ONE_ANT:" + String(one_ant));
				step = 1;
			}

			else if (reader_cmd == "24")
				step = 2;

			else if (reader_cmd == "22")
				step = 3;

			else if (reader_cmd == "76")
				step = 4;

			else if (reader_cmd == "75")
				step = 5;

			else if (reader_cmd == "ea")
			{
				if (step == 5)
					step = 6;
				else if (step == 6)
					step = 7;
				else if (step == 11)
					step = 12;
			}

			else if (reader_cmd == "66")
				step = 8;

			else if (reader_cmd == "25")
				step = 9;

			else if (reader_cmd == "48")
				step = 10;

			else if (reader_cmd == "7b")
				step = 11;

			else if (reader_cmd == "79")
				step = 13;

			else if (reader_cmd == "7f")
			{
				if (step == 13)
					step = 14;
				else if (step == 14)
					step = 15;
			}

			else if (reader_cmd == "3f")
				step = 16;

			else if (reader_cmd == "2f")
				step = 17;

			// ACTIONS

			else if (reader_cmd == "01" && cmd.substring(6, 8) == "f8")
			{
				myserial.write("#ANT_ERROR: ");
			}

			else if (reader_cmd == "06")
			{
				if (cmd.substring(6, 8) == "00")
				{
					myserial.write("#LOCK:OK");
				}
				else
				{
					myserial.write("#LOCK:ERROR");
				}
			}

			// WRITE
			else if (reader_cmd == "03" || reader_cmd == "04")
			{
				if (cmd.substring(6, 8) == "00")
				{
					myserial.write("#TAG_WRITE:OK");
				}
				else
				{
					myserial.write("#TAG_WRITE:ERROR");
				}
				tag_commands.clear_tags();
			}

			// PROTECTED MODE
			else if (reader_cmd == "e9")
			{
				if (cmd.substring(6, 8) == "00")
				{
					myserial.write("#TAG_PROTECTED:OK");
				}
				else
				{
					myserial.write("#TAG_PROTECTED:ERROR");
				}
			}
		}
		else
		{
			return;
		}

		if (reader_cmd == "01")
		{
			tag_command(cmd);
		}
		else if (reader_cmd == "92")
		{
			String current_temp = cmd.substring(cmd.length() - 6, cmd.length() - 4);
			temperatura = (byte)strtol(current_temp.c_str(), NULL, 16);
			myserial.write("#TEMPERATURA:" + String(temperatura, DEC));
		}
	}

	void tag_command(String tag_cmd)
	{
		if (tag_cmd.length() < 18)
			return;
		if (!is_hex_string(tag_cmd))
			return;

		const int cmd_size = strtol(tag_cmd.substring(0, 2).c_str(), NULL, 16);
		const int cmd_len_chars = (cmd_size + 1) * 2;
		if (cmd_size <= 0 || cmd_len_chars != tag_cmd.length())
			return;

		// Expected frame format (byte-by-byte):
		// [size] [00] [01] [xx] [xx] [xx] [pc0] [pc1] [epc...] [tid...] [rssi]
		if (tag_cmd.substring(4, 6) != "01")
			return;

		// Rule requested: when size = 0x21 => EPC = 24 chars.
		const int epc_len = (cmd_size - 0x15) * 2;
		if (epc_len < 4 || epc_len > 48 || (epc_len % 2) != 0 || (epc_len % 4) != 0)
			return;

		const int epc_start = 14;
		const int epc_end = epc_start + epc_len;
		if (epc_end > tag_cmd.length())
			return;

		const String current_epc = tag_cmd.substring(epc_start, epc_end);
		if (!is_hex_string(current_epc))
			return;

		const int tid_start = epc_end;
		if (tid_start + 24 > tag_cmd.length() - 2)
			return;

		const String current_tid = tag_cmd.substring(tid_start, tid_start + 24);
		if (!is_hex_string(current_tid))
			return;
		if (current_tid.substring(0, 2) != "e2")
			return;

		int current_rssi = 0;
		if (tag_cmd.length() >= 2)
			current_rssi = strtol(tag_cmd.substring(tag_cmd.length() - 6, tag_cmd.length() - 4).c_str(), NULL, 16);
		current_rssi = abs(128 - current_rssi);

		int current_ant = 0;
		const int ant_positions[3] = {6, 8, 10};
		for (int i = 0; i < 3; i++)
		{
			const int pos = ant_positions[i];
			if (tag_cmd.length() < pos + 2)
				continue;

			const int ant_candidate = strtol(tag_cmd.substring(pos, pos + 2).c_str(), NULL, 16);
			if (ant_candidate == 1 || ant_candidate == 2 || ant_candidate == 4 || ant_candidate == 8)
			{
				current_ant = ant_candidate;
				break;
			}
		}

		if (current_ant == 0)
			current_ant = 1;

		if (current_ant == 4)
			current_ant = 3;
		if (current_ant == 8)
			current_ant = 4;

		tag_commands.add_tag(current_epc, current_tid, current_ant, current_rssi);
	}
};
