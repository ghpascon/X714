struct TagItem
{
    String epc;
    String tid;
    byte ant;
    byte rssi;
    bool protected_tag;
};

class Tags
{
public:
    std::vector<TagItem> _tags;

    // Adiciona um novo tag se tid não existir
    bool add(String epc, String tid, byte ant, byte rssi, bool protected)
    {
        // RSSI
        if (rssi < settings.current_rssi)
            return false;

        // PREFIX
        if (!validate_prefix(epc))
            return false;

        // GTIN
        if (settings.decode_gtin)
        {
            String gtin = epcToGtin(epc);
            if (gtin == "")
                return false;
        }

        // Mount message
        String tag_message = "#T+@" + (settings.decode_gtin ? gtin : epc) + "|" + tid + "|" + ant + "|" + rssi + "|" + (protected ? "on" : "off");
        if (settings.simple_send)
            tag_message = (settings.decode_gtin ? gtin : epc);

        if (settings.always_send)
            myserial.send(tag_message);

        // DUPLICATE TID
        for (const auto &tag : _tags)
        {
            if (tag.tid == tid)
                return false; // já existe
        }
        _tags.push_back({epc, tid, ant, rssi, protected});
        if (!settings.always_send)
            myserial.send(tag_message);
        return true;
    }

    // Retorna todos os tags
    std::vector<TagItem> get() const
    {
        return _tags;
    }

    // Limpa todos os tags
    void clear()
    {
        _tags.clear();
    }

    int count() const
    {
        return _tags.size();
    }

private:
    // prefixes: string separada por vírgula
    bool validate_prefix(String epc)
    {
        String prefixes = settings.prefix;
        int start = 0;
        while (true)
        {
            int end = prefixes.indexOf(',', start);
            String prefix = (end == -1) ? prefixes.substring(start) : prefixes.substring(start, end);
            prefix.trim();
            if (prefix.length() > 0 && epc.startsWith(prefix))
                return true;
            if (end == -1)
                break;
            start = end + 1;
        }
        return false;
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