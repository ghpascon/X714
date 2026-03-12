#include "vars.h"
#include <freertos/semphr.h>

class TAG_COMMANDS
{
public:
	bool send_protect_mode = false;
	void ensure_protect_mode_correct()
	{
		static unsigned long current_time = 0;
		if (!reader_in_protected_inventory)
			current_time = millis();
		if (millis() - current_time > 500)
		{
			send_protect_mode = true;
		}
		else
		{
			send_protect_mode = false;
		}
	}

	void add_tag(String current_epc, String current_tid, int current_ant, int current_rssi)
	{
		// quick checks that don't need tags[] lock
		if (!read_on)
			return;

		if (antena[current_ant - 1].rssi < current_rssi)
			return;

		// prefix implementation
		if (prefix.length() > 0)
		{
			bool prefix_match = false;

			// Se não tem vírgula, testa prefixo único
			if (prefix.indexOf(',') == -1)
			{
				String trimmed_prefix = prefix;
				trimmed_prefix.trim();
				trimmed_prefix.toLowerCase();
				if (trimmed_prefix.length() > 0 && current_epc.startsWith(trimmed_prefix))
				{
					prefix_match = true;
				}
			}
			else
			{
				// Divide os prefixes por vírgula e verifica cada um
				int start = 0;
				int separator_pos = prefix.indexOf(',');

				while (start < prefix.length())
				{
					String current_prefix = (separator_pos != -1) ? prefix.substring(start, separator_pos) : prefix.substring(start);

					current_prefix.trim(); // Remove espaços
					current_prefix.toLowerCase();
					if (current_prefix.length() > 0 && current_epc.startsWith(current_prefix))
					{
						prefix_match = true;
						break;
					}

					if (separator_pos == -1)
						break;

					start = separator_pos + 1;
					separator_pos = prefix.indexOf(',', start);
				}
			}

			if (!prefix_match)
				return;
		}

		if (always_send)
			display_current_tag(String(current_epc), String(current_tid), String(current_ant), String(current_rssi));

		// Check duplicates and find empty slot
		for (int i = 0; i < max_tags; i++)
		{
			if (current_epc == tags[i].epc || current_tid == tags[i].tid)
				return;
			if (tags[i].epc == "" || tags[i].tid == "")
				break;
		}

		tags[current_tag].epc = current_epc;
		tags[current_tag].tid = current_tid;
		tags[current_tag].ant_number = current_ant;
		tags[current_tag].rssi = current_rssi;

		current_tag_num();

		if (buzzer_on)
			pins.buzzer_time = millis();

		if (!always_send)
			display_current_tag(String(tags[(current_tag == 0 ? max_tags - 1 : current_tag - 1)].epc), String(tags[(current_tag == 0 ? max_tags - 1 : current_tag - 1)].tid), String(tags[(current_tag == 0 ? max_tags - 1 : current_tag - 1)].ant_number), String(tags[(current_tag == 0 ? max_tags - 1 : current_tag - 1)].rssi));
	}

	void tag_data_display()
	{
		String tags_to_send = "#tags:";
		for (int i = 0; i < max_tags; i++)
		{
			if (tags[i].epc == "")
				break;
			tags_to_send += "@" + tags[i].epc;
		}
		myserial.write(tags_to_send, true);

		clear_tags();
	}

	void tag_data_display_all()
	{
		String tags_to_send = "#tags:";
		for (int i = 0; i < max_tags; i++)
		{
			if (tags[i].epc == "")
				break;
			tags_to_send += "@" + tags[i].epc + "|" + tags[i].tid + "|" + tags[i].ant_number + "|" + tags[i].rssi;
		}
		myserial.write(tags_to_send, true);
		clear_tags();
	}

	void clear_tags()
	{
		for (int i = 0; i < max_tags; i++)
		{
			tags[i].epc = "";
			tags[i].tid = "";
			tags[i].ant_number = 0;
			tags[i].rssi = 0;
		}
		current_tag = 0;
		myserial.write("#TAGS_CLEARED");
	}

private:
	void current_tag_num()
	{
		(current_tag < max_tags - 1) ? current_tag++ : current_tag = 0;
	}

	void display_current_tag(String epc, String tid, String ant, String rssi)
	{
		if (decode_gtin)
			epc = epcToGtin(epc);
		if (epc == "")
			return;
		if (!simple_send)
			myserial.write("#T+@" + epc + "|" + tid + "|" + ant + "|" + rssi + "|" + (send_protect_mode ? "on" : "off"));
		else
			myserial.write(epc, true);
	}

	// Calculate GTIN check digit (mod 10)
	int calcCheckDigit(const String &number)
	{
		int sum = 0;
		bool multiplyBy3 = true;
		for (int i = number.length() - 1; i >= 0; i--)
		{
			int n = number[i] - '0';
			sum += multiplyBy3 ? n * 3 : n;
			multiplyBy3 = !multiplyBy3;
		}
		return (10 - (sum % 10)) % 10;
	}

	// Convert EPC (SGTIN-96) → GTIN-14
	String epcToGtin(const String &epcHex)
	{
		if (epcHex.length() != 24)
			return "";

		// Convert hex string → 12 bytes
		uint8_t epc[12];
		for (int i = 0; i < 12; i++)
		{
			String byteStr = epcHex.substring(i * 2, i * 2 + 2);
			epc[i] = (uint8_t)strtoul(byteStr.c_str(), nullptr, 16);
		}

		// ✅ Check header for SGTIN-96 (0x30)
		if (epc[0] != 0x30)
			return "";

		// Build 96-bit string
		String bits = "";
		for (int i = 0; i < 12; i++)
		{
			for (int b = 7; b >= 0; b--)
			{
				bits += ((epc[i] >> b) & 1) ? '1' : '0';
			}
		}

		// Skip header(8) + filter(3) + partition(3) = 14 bits
		int pos = 14;

		// Partition 5 → Company Prefix = 7 digits (24 bits), Item Reference = 6 digits (20 bits)
		int cpBits = 24;
		int irBits = 20;
		int cpDigits = 7;
		int irDigits = 6;

		// Extract company prefix
		String cpBin = bits.substring(pos, pos + cpBits);
		pos += cpBits;
		uint64_t company = strtoull(cpBin.c_str(), nullptr, 2);

		// Extract item reference
		String irBin = bits.substring(pos, pos + irBits);
		uint64_t item = strtoull(irBin.c_str(), nullptr, 2);

		// Pad company + item
		char buf[32];
		snprintf(buf, sizeof(buf), "%0*llu%0*llu",
				 cpDigits, (unsigned long long)company,
				 irDigits, (unsigned long long)item);

		String full = String(buf);

		// Safety: must have correct length
		if (full.length() != (cpDigits + irDigits))
			return "";

		// Split item reference → indicator + remaining
		String indicator = full.substring(cpDigits, cpDigits + 1);
		String itemRest = full.substring(cpDigits + 1);
		String companyStr = full.substring(0, cpDigits);

		// Construct GTIN-13 without check digit
		String base = indicator + companyStr + itemRest;

		// Must be 13 digits
		if (base.length() != 13)
			return "";

		// Reject all zeros
		if (base == "0000000000000")
			return "";

		// Add check digit
		int check = calcCheckDigit(base);
		String gtin = base + String(check);

		// Final validation
		if (gtin.length() != 14)
			return "";
		if (gtin == "00000000000000")
			return "";

		if (gtin[0] == '0')
			gtin = gtin.substring(1);
		return gtin;
	}
};
