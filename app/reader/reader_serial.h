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
			myserial.write("#ERRO: " + status);
			return;
		}

		// temperatura
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
		const String tag_data_prefix = "21000103";
		while (true)
		{
			const int start_tag_data = tag_cmd.indexOf(tag_data_prefix);
			if (start_tag_data == -1)
				break;

			tag_cmd = tag_cmd.substring(start_tag_data + tag_data_prefix.length());
			if (tag_cmd.length() < 57)
				break;

			if (tag_cmd.substring(4, 6).toInt() != 98)
				continue;

			const String current_epc = tag_cmd.substring(6, 30);
			const String current_tid = tag_cmd.substring(30, 54);

			int current_ant = tag_cmd.substring(0, 2).toInt();
			if (current_ant == 4)
				current_ant = 3;
			if (current_ant == 8)
				current_ant = 4;

			const int current_rssi = strtol(tag_cmd.substring(54, 56).c_str(), NULL, 16);
			tag_commands.add_tag(current_epc, current_tid, current_ant, current_rssi);
		}
	}
};
