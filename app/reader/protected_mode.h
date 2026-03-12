class protected_mode : public commands_reader
{
public:
    void protected_mode_tag(String epc, String password, bool enable)
    {
        // Validate input parameters
        if (epc.length() != 24 || password.length() != 8)
        {
            return; // Invalid parameters
        }

        // Convert EPC hex string to bytes (12 bytes from 24 hex chars)
        byte epc_bytes[12];
        for (int i = 0; i < 12; i++)
        {
            String byteStr = epc.substring(i * 2, i * 2 + 2);
            epc_bytes[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
        }

        // Convert password hex string to bytes (4 bytes from 8 hex chars)
        byte password_bytes[4];
        for (int i = 0; i < 4; i++)
        {
            String byteStr = password.substring(i * 2, i * 2 + 2);
            password_bytes[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
        }

        // Build command array with proper structure
        byte reader_protected_mode_command[] = {
            0x16,
            0xff,
            0xe9,
            0x06,
            // EPC bytes (12 bytes)
            epc_bytes[0],
            epc_bytes[1], epc_bytes[2], epc_bytes[3],
            epc_bytes[4], epc_bytes[5], epc_bytes[6], epc_bytes[7],
            epc_bytes[8], epc_bytes[9], epc_bytes[10], epc_bytes[11],
            // Enable/disable flag
            enable ? 0x01 : 0x00,
            // Password bytes (4 bytes)
            password_bytes[0], password_bytes[1], password_bytes[2], password_bytes[3]};

        // Calculate CRC and send command
        crcValue = uiCrc16Cal(reader_protected_mode_command, sizeof(reader_protected_mode_command));
        crc1 = crcValue & 0xFF;
        crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(reader_protected_mode_command, sizeof(reader_protected_mode_command), crc1, crc2, false);
    }

    void protected_inventory(bool enable, String password = "00000000")
    {
        if (!validateHex(password, 8))
        {
            password = "00000000";
        }

        // Password → bytes
        byte password_bytes[4];
        for (int i = 0; i < 4; i++)
        {
            String byteStr = password.substring(i * 2, i * 2 + 2);
            password_bytes[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
        }

        // Enable byte
        byte enable_byte = enable ? 0x01 : 0x00;

        // TARGET = XOR(enable + password bytes)
        byte target =
            enable_byte ^
            password_bytes[0] ^
            password_bytes[1] ^
            password_bytes[2] ^
            password_bytes[3];

        // Command
        byte protected_inventory_command[] = {
            0x0c,
            0xff,
            0xea,
            0x00,
            0x0e,
            enable_byte,
            password_bytes[0],
            password_bytes[1],
            password_bytes[2],
            password_bytes[3],
            target};

        crcValue = uiCrc16Cal(
            protected_inventory_command,
            sizeof(protected_inventory_command));

        crc1 = crcValue & 0xFF;
        crc2 = (crcValue >> 8) & 0xFF;

        write_bytes(
            protected_inventory_command,
            sizeof(protected_inventory_command),
            crc1,
            crc2,
            false);

        reader_in_protected_inventory = enable;
    }
};
