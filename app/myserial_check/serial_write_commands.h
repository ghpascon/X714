class serial_port_write
{
public:
    void write_tag_cmd(String cmd)
    {
        if (!cmd.startsWith("#write:"))
        {
            myserial.write("#ERROR:Invalid command prefix\n");
            return;
        }

        String payload = cmd.substring(7); // remove prefix

        // Split by ';' into max 4 parts
        String parts[4];
        int count = 0;
        int start = 0;
        for (int i = 0; i < payload.length() && count < 4; i++)
        {
            if (payload.charAt(i) == ';')
            {
                parts[count++] = payload.substring(start, i);
                start = i + 1;
            }
        }
        parts[count++] = payload.substring(start); // last part

        // Validate number of parts
        if (count < 2)
        {
            myserial.write("#ERROR:Missing EPC or password\n");
            return;
        }
        if (count > 4)
        {
            myserial.write("#ERROR:Too many separators\n");
            return;
        }

        String newEPC = parts[0];
        String password = parts[1];
        String targetType = (count >= 3) ? parts[2] : "";
        String targetValue = (count == 4) ? parts[3] : "";

        // Validate EPC
        if (!validateHex(newEPC, 24))
        {
            myserial.write("#ERROR:Invalid EPC length\n");
            return;
        }

        // Validate password
        if (!validateHex(password, 8))
        {
            myserial.write("#ERROR:Invalid password\n");
            return;
        }

        // Validate optional target
        if (targetType.length() > 0)
        {
            targetType.toLowerCase();
            if ((targetType != "epc" && targetType != "tid") || !validateHex(targetValue, 24))
            {
                myserial.write("#ERROR:Invalid target type/value\n");
                return;
            }
        }

        // Convert hex strings to byte arrays
        byte epc_bytes[12], pwd_bytes[4], mask_bytes[12];
        hexToBytes(newEPC, epc_bytes, 12);
        hexToBytes(password, pwd_bytes, 4);

        if (targetType.length() > 0)
        {
            hexToBytes(targetValue, mask_bytes, 12);
            reader_module.write_tag(epc_bytes, pwd_bytes, targetType, mask_bytes);
        }
        else
        {
            reader_module.write_tag_no_filter(epc_bytes, pwd_bytes);
        }
    }

private:
    void hexToBytes(String hex, byte *buffer, int bufferLen)
    {
        for (int i = 0; i < bufferLen; i++)
        {
            String byteStr = hex.substring(i * 2, i * 2 + 2);
            buffer[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
        }
    }
};
