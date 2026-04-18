class serial_reader
{
public:
	void check_serial()
	{
		if (!Serial2.available())
			return;

		const int timeout_serial_rec = 100;
		unsigned long current_timeout_serial_rec = 0;
		const int full_cmd_timeout = 3000;
		unsigned long current_full_cmd_timeout = millis();
		int cmd_length = 0;
		String cmd_rec = "";
		current_timeout_serial_rec = millis();
		while (millis() - current_timeout_serial_rec < timeout_serial_rec && millis() - current_full_cmd_timeout < full_cmd_timeout)
		{
			if (!Serial2.available())
				continue;
			byte cmd = Serial2.read();

			// determine expected command length from first byte
			if (cmd_rec == "")
			{
				cmd_length = 2 * (cmd + 1);
				answer_rec = true; // Reset answer_rec for each new command
			}

			cmd_rec += String((cmd < 0x10) ? "0" : "") + String(cmd, HEX);

			// process command when complete
			if (cmd_rec.length() == cmd_length)
			{
				cmd_handler(cmd_rec);
				cmd_rec = "";
				cmd_length = 0;
				// answer_rec will be set to false by write_bytes in cmd_handler
			}

			current_timeout_serial_rec = millis();
		}

		// Process any remaining incomplete command
		if (cmd_rec.length() > 0)
		{
			cmd_handler(cmd_rec);
		}
	}

private:
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

		String status = cmd.substring(2, 4);
		String reader_cmd = cmd.substring(4, 6);

		if (status == "00")
		{
			if (reader_cmd == "01" && cmd.substring(6, 8) == "f8")
			{
				myserial.write("#ANT_ERROR: ");
			}

			if (reader_cmd == "21")
			{
				if (cmd.substring(12, 14) == "71")
					one_ant = true;
				else
					one_ant = false;
				myserial.write("#ONE_ANT:" + String(one_ant));
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

			if (!setup_done)
			{
				step++;
				myserial.write("#STEP:" + String(step));
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
		// Split concatenated inventory data using each frame length byte (hex).
		while (tag_cmd.length() >= 2)
		{
			const int cmd_size = strtol(tag_cmd.substring(0, 2).c_str(), NULL, 16);
			const int cmd_len_chars = (cmd_size + 1) * 2;

			if (cmd_size <= 0)
			{
				tag_cmd = tag_cmd.substring(2);
				continue;
			}

			if (cmd_len_chars > tag_cmd.length())
			{
				tag_cmd = tag_cmd.substring(2);
				continue;
			}

			handle_tag_command(tag_cmd.substring(0, cmd_len_chars));
			tag_cmd = tag_cmd.substring(cmd_len_chars);
		}
	}

	void handle_tag_command(String tag_cmd)
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
		if (tag_cmd.substring(2, 4) != "00")
			return;
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
			current_rssi = strtol(tag_cmd.substring(tag_cmd.length() - 2).c_str(), NULL, 16);

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
