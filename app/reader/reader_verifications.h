class reader_verifications : public setup_commands_reader
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
		else
		{
			clear_buffer();
		}
	}

	void check_timeout(bool force = false)
	{
		const int answer_timeout = 200;
		static unsigned long current_answer_timeout = 0;
		if (setup_done && read_on && (millis() - current_answer_timeout > answer_timeout))
			answer_rec = true;
		if (answer_rec)
			current_answer_timeout = millis();

		if (force || (millis() - current_answer_timeout > answer_timeout && setup_done))
		{
			myserial.write("#TIMEOUT");
			answer_rec = true;
			setup_done = false;
			step = 0;
			expected_setup_ack_cmd = 0x00;
			last_wait_cmd_sent_ms = 0;
			setup_transition_until_ms = millis() + 120;
			// Flush stale UART bytes so they don't corrupt the first frame
			// of the new setup sequence.
			request_clear_serial_buffers = true;
		}
	}

	void try_change_baudrate()
	{
		const int max_reconnect_attempts = 3;
		// If the reader has already exchanged valid frames at 115200, the
		// disconnection is transient — not a baudrate mismatch.
		// Reinitialise Serial2 at the current baud and let normal setup retry.
		// After several failed reconnects, restart the ESP32 as a last resort.
		if (had_valid_frame)
		{
			if (reconnect_count > max_reconnect_attempts)
			{
				myserial.write("#RECONNECT_FAILED: restart");
				delay(100);
				ESP.restart();
			}

			reconnect_count++;
			myserial.write("#RECONNECT_115200 (" + String(reconnect_count) + "/" + String(max_reconnect_attempts) + ")");
			Serial2.end();
			delay(50);
			Serial2.begin(115200, SERIAL_8N1, rx_reader_module, tx_reader_module);
			delay(100);
			while (Serial2.available())
				Serial2.read();
			return;
		}

		// Sends baud-change command at the old baud, waits for response and reports result.
		myserial.write("#TRY_CHANGE_BAUDRATE");

		const int BAUD_OLD = 57600;					   // current baud
		const int BAUD_NEW = 115200;				   // desired baud
		const int BAUD_PARAM = 0x06;				   // per docs: 6 => 115200
		const unsigned long RESPONSE_TIMEOUT_MS = 800; // wait for response at old baud
		const unsigned long BETWEEN_BYTE_TIMEOUT_MS = 100;
		const int MAX_ATTEMPTS = 2;

		byte start[] = {0x05, 0xff, 0x28, (byte)BAUD_PARAM};

		// Try a couple times to be more robust
		for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt)
		{
			// Ensure serial is at the old baud to send command and receive reply
			Serial2.end();
			delay(100);
			Serial2.begin(BAUD_OLD, SERIAL_8N1, rx_reader_module, tx_reader_module);
			delay(200);

			// Drain any stray bytes
			while (Serial2.available())
				Serial2.read();

			// Prepare CRC and send frame (don't toggle global answer_rec here)
			crcValue = uiCrc16Cal(start, sizeof(start));
			crc1 = crcValue & 0xFF;
			crc2 = (crcValue >> 8) & 0xFF;
			write_bytes(start, sizeof(start), crc1, crc2, false);
			Serial2.flush();

			// Wait and parse incoming frame (simple one-shot parser similar to check_serial())
			byte rx_buffer[64];
			int rx_size = 0;
			int expected_size = 0;
			unsigned long start_ms = millis();
			unsigned long last_byte_ms = 0;
			bool got_valid_response = false;

			while (millis() - start_ms < RESPONSE_TIMEOUT_MS)
			{
				if (Serial2.available())
				{
					byte cur = Serial2.read();
					unsigned long now = millis();
					if (rx_size == 0)
					{
						expected_size = ((int)cur) + 1; // frames use len + 1 convention
						if (expected_size < 5 || expected_size > (int)sizeof(rx_buffer))
						{
							expected_size = 0; // invalid start, discard
							continue;
						}
						// store first byte
					}

					rx_buffer[rx_size++] = cur;
					last_byte_ms = now;

					if (expected_size > 0 && rx_size == expected_size)
					{
						// validate CRC
						uint16_t crc = uiCrc16Cal(rx_buffer, rx_size - 2);
						byte rcrc1 = crc & 0xFF;
						byte rcrc2 = (crc >> 8) & 0xFF;
						if (rx_buffer[rx_size - 2] == rcrc1 && rx_buffer[rx_size - 1] == rcrc2)
						{
							// Check it's the response for cmd 0x28 and status == 0x00
							if (rx_size >= 5 && rx_buffer[2] == 0x28 && rx_buffer[1] == 0x00)
							{
								got_valid_response = true;
							}
						}

						break; // stop waiting once a frame is complete (valid or not)
					}
				}
				else
				{
					delay(5);
					// optional: break if a long gap between bytes
					if (rx_size > 0 && (millis() - last_byte_ms) > BETWEEN_BYTE_TIMEOUT_MS)
						break;
				}
			}

			if (got_valid_response)
			{
				// Switch host to new baud so next communications use it
				Serial2.end();
				delay(100);
				Serial2.begin(BAUD_NEW, SERIAL_8N1, rx_reader_module, tx_reader_module);
				delay(200);
				// drain any garbage after switching
				while (Serial2.available())
					Serial2.read();

				myserial.write("change_baudrate_ok");
				return;
			}
			else
			{
				// try again unless this was the last attempt
				if (attempt + 1 < MAX_ATTEMPTS)
				{
					myserial.write("change_baudrate_retry");
					delay(200);
					continue;
				}
				else
				{
					myserial.write("change_baudrate_nok");
					ESP.restart(); // restart to try again on next boot
				}
			}
		}
	}

	void check_reader_connection()
	{
		const int timeout_reader_connection = 4000;
		static unsigned long current_timeout_reader_connection = 0;
		if (setup_done)
		{
			current_timeout_reader_connection = millis();
			reconnect_count = 0; // reset on healthy setup
		}

		// Don't run reconnect logic while setup is actively transitioning or
		// waiting for a step ACK. Let the setup watchdog/resend path handle it.
		if (!setup_done && (setup_transition_until_ms > 0 || expected_setup_ack_cmd != 0x00))
			return;

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
