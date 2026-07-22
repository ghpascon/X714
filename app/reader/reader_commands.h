class commands_reader
{
protected:
	unsigned long cmd_sent_ms = 0; // timestamp of last command expecting a response

public:
	unsigned int uiCrc16Cal(unsigned char const *pucY, unsigned char ucX)
	{
		unsigned char ucI, ucJ;
		unsigned short int uiCrcValue = PRESET_VALUE;

		for (ucI = 0; ucI < ucX; ucI++)
		{
			uiCrcValue = uiCrcValue ^ *(pucY + ucI);
			for (ucJ = 0; ucJ < 8; ucJ++)
			{
				if (uiCrcValue & 0x0001)
				{
					uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
				}
				else
				{
					uiCrcValue = (uiCrcValue >> 1);
				}
			}
		}
		return uiCrcValue;
	}

	void write_bytes(byte values[], byte lenght, byte crc1, byte crc2, bool wait_answer = true)
	{
		if (debug_mode)
		{
			String dbg = "[ DEBUG ] -> ";
			for (int i = 0; i < lenght; i++)
				dbg += (values[i] < 0x10 ? "0" : "") + String(values[i], HEX);
			dbg += (crc1 < 0x10 ? "0" : "") + String(crc1, HEX);
			dbg += (crc2 < 0x10 ? "0" : "") + String(crc2, HEX);
			myserial.write(dbg);
		}
		for (int i = 0; i < lenght; i++)
			Serial2.write(values[i]);
		Serial2.write(crc1);
		Serial2.write(crc2);
		if (wait_answer)
		{
			answer_rec = false;
			cmd_sent_ms = millis();
			last_wait_cmd_sent_ms = cmd_sent_ms;
		}
	}
};
