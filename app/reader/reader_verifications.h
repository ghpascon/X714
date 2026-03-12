class reader_verifications : public commands_reader
{
public:
	void reader_verify()
	{
		check_read();
		check_timeout();
		check_reader_connection();
		reset_no_read();
	}

	void check_read()
	{
		static bool last_read = false;
		if (last_read == read_on)
			return;

		last_read = read_on;

		myserial.write("#READ:" + String(read_on ? "ON" : "OFF"));

		if (read_on)
		{
			tag_commands.clear_tags();
		}
	}

	void check_timeout(bool force = false)
	{
		const int answer_timeout = 1000;
		static bool first_timeout = true;
		static unsigned long current_answer_timeout = 0;
		if (answer_rec)
			current_answer_timeout = millis();

		if (force || (millis() - current_answer_timeout > answer_timeout && setup_done))
		{
			if (!first_timeout)
			{
				antena_commands.decrease_power();
			}
			myserial.write("#TIMEOUT");
			answer_rec = true;
			first_timeout = false;
			setup_done = false;
			step = 0;
		}
	}

	void try_change_baudrate()
	{
		Serial2.end();
		delay(100);
		Serial2.begin(57600, SERIAL_8N1, rx_reader_module, tx_reader_module);
		delay(300);
		byte start[] = {0x05, 0xff, 0x28, 0x06};
		crcValue = uiCrc16Cal(start, sizeof(start));
		crc1 = crcValue & 0xFF;
		crc2 = (crcValue >> 8) & 0xFF;
		write_bytes(start, sizeof(start), crc1, crc2);
		Serial2.end();
		delay(100);
		Serial2.begin(115200, SERIAL_8N1, rx_reader_module, tx_reader_module);
		delay(300);
	}

	void check_reader_connection()
	{
		const int timeout_reader_connection = 5000;
		static unsigned long current_timeout_reader_connection = 0;
		if (setup_done)
			current_timeout_reader_connection = millis();
		if (millis() - current_timeout_reader_connection > timeout_reader_connection)
		{
			try_change_baudrate();
			check_timeout(true);
			current_timeout_reader_connection = millis();
		}
	}

	void reset_no_read()
	{
		const int timeout_no_read = 1800000;
		static unsigned long current_timeout_no_read = 0;
		if (read_on)
			current_timeout_no_read = millis();
		if (millis() - current_timeout_no_read > timeout_no_read)
			ESP.restart();
	}
};
