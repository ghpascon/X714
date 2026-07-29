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

        // Validate EPC: only check hex chars and that length is multiple of 4 (words)
        if (!validateHex(newEPC, newEPC.length()) || (newEPC.length() % 4 != 0))
        {
            myserial.write("#ERROR:Invalid EPC (must be hex and multiple of 4 chars)\n");
            return;
        }

        // Validate password: only check hex chars and that length is multiple of 4 (words)
        if (!validateHex(password, password.length()) || (password.length() % 4 != 0))
        {
            myserial.write("#ERROR:Invalid password (must be hex and multiple of 4 chars)\n");
            return;
        }

        // Validate optional target
        if (targetType.length() > 0)
        {
            targetType.toLowerCase();
            if ((targetType != "epc" && targetType != "tid") || !validateHex(targetValue, targetType == "tid" ? 24 : targetValue.length()) || (targetValue.length() % 4 != 0))
            {
                myserial.write("#ERROR:Invalid target type/value (must be hex and multiple of 4 chars)\n");
                return;
            }
        }

        if (targetType.length() > 0)
        {
            reader_module.write_tag(newEPC, password, targetType, targetValue);
        }
        else
        {
            reader_module.write_tag_no_filter(newEPC, password);
        }
    }
};
