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

        const int p1 = payload.indexOf(';');
        if (p1 == -1)
        {
            myserial.write("#ERROR:Missing EPC or password\n");
            return;
        }

        const int p2 = payload.indexOf(';', p1 + 1);
        const int p3 = (p2 == -1) ? -1 : payload.indexOf(';', p2 + 1);
        if (p3 != -1 && payload.indexOf(';', p3 + 1) != -1)
        {
            myserial.write("#ERROR:Too many separators\n");
            return;
        }

        String newEPC = payload.substring(0, p1);
        String password = (p2 == -1) ? payload.substring(p1 + 1) : payload.substring(p1 + 1, p2);
        String targetType = (p2 == -1) ? "" : ((p3 == -1) ? payload.substring(p2 + 1) : payload.substring(p2 + 1, p3));
        String targetValue = (p3 == -1) ? "" : payload.substring(p3 + 1);

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
