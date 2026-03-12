class reader_write_commands : public commands_reader
{
public:
    // Write tag using EPC or TID with optional mask
    void write_tag(byte *new_epc, byte *password, String targetType, byte *mask_bytes = nullptr, byte *current_epc = nullptr)
    {
        targetType == "epc" ? write_tag_epc(mask_bytes, new_epc, password) : write_tag_tid(new_epc, password, mask_bytes);
    }

    // Write TID with mask
    void write_tag_tid(byte *new_epc, byte *password, byte *mask_bytes)
    {
        byte reader_write_command[] = {
            0x28,
            0xff,
            0x03,
            0x06,
            0xff,

            0x01,
            0x02,

            new_epc[0],
            new_epc[1],
            new_epc[2],
            new_epc[3],
            new_epc[4],
            new_epc[5],
            new_epc[6],
            new_epc[7],
            new_epc[8],
            new_epc[9],
            new_epc[10],
            new_epc[11],

            password[0],
            password[1],
            password[2],
            password[3],

            0x02,

            0x00,
            0x00,
            0x60,

            mask_bytes[0],
            mask_bytes[1],
            mask_bytes[2],
            mask_bytes[3],
            mask_bytes[4],
            mask_bytes[5],
            mask_bytes[6],
            mask_bytes[7],
            mask_bytes[8],
            mask_bytes[9],
            mask_bytes[10],
            mask_bytes[11],
        };

        uint16_t crcValue = uiCrc16Cal(reader_write_command, sizeof(reader_write_command));
        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(reader_write_command, sizeof(reader_write_command), crc1, crc2);
    }

    // Write EPC
    void write_tag_epc(byte *current_epc, byte *new_epc, byte *password)
    {
        byte reader_write_command[] = {
            0x24,
            0xff,
            0x03,
            0x06,
            0x06,

            current_epc[0], current_epc[1], current_epc[2], current_epc[3],
            current_epc[4], current_epc[5], current_epc[6], current_epc[7],
            current_epc[8], current_epc[9], current_epc[10], current_epc[11],

            0x01,
            0x02,

            new_epc[0], new_epc[1], new_epc[2], new_epc[3],
            new_epc[4], new_epc[5], new_epc[6], new_epc[7],
            new_epc[8], new_epc[9], new_epc[10], new_epc[11],

            password[0], password[1], password[2], password[3]};

        uint16_t crcValue = uiCrc16Cal(reader_write_command, sizeof(reader_write_command));
        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(reader_write_command, sizeof(reader_write_command), crc1, crc2);
    }

    // Write EPC without any filter
    void write_tag_no_filter(byte *new_epc, byte *password)
    {
        byte reader_write_command[] = {
            0x15,
            0xff,
            0x04,
            0x06,

            password[0],
            password[1],
            password[2],
            password[3],

            new_epc[0],
            new_epc[1],
            new_epc[2],
            new_epc[3],
            new_epc[4],
            new_epc[5],
            new_epc[6],
            new_epc[7],
            new_epc[8],
            new_epc[9],
            new_epc[10],
            new_epc[11],
        };

        uint16_t crcValue = uiCrc16Cal(reader_write_command, sizeof(reader_write_command));
        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(reader_write_command, sizeof(reader_write_command), crc1, crc2);
    }

    // Change password command parser
    void change_password(String epc, String new_password, String old_password = "00000000")
    {
        // Validate input parameters
        if (epc.length() != 24 || new_password.length() != 8 || old_password.length() != 8)
        {
            return; // Invalid parameters
        }

        // Validate hex strings
        if (!validateHex(epc, 24) || !validateHex(new_password, 8) || !validateHex(old_password, 8))
        {
            return; // Invalid hex format
        }

        // Convert EPC hex string to bytes (12 bytes from 24 hex chars)
        byte epc_bytes[12];
        for (int i = 0; i < 12; i++)
        {
            String byteStr = epc.substring(i * 2, i * 2 + 2);
            epc_bytes[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
        }

        // Convert new password hex string to bytes (4 bytes from 8 hex chars)
        byte new_password_bytes[4];
        for (int i = 0; i < 4; i++)
        {
            String byteStr = new_password.substring(i * 2, i * 2 + 2);
            new_password_bytes[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
        }

        // Convert old password hex string to bytes (4 bytes from 8 hex chars)
        byte old_password_bytes[4];
        for (int i = 0; i < 4; i++)
        {
            String byteStr = old_password.substring(i * 2, i * 2 + 2);
            old_password_bytes[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
        }

        // Build command array: 1c 00 03 02 06 epc 00 02 new_password old_password
        byte change_password_command[] = {
            0x1c,
            0x00,
            0x03,
            0x02,
            0x06,
            // EPC bytes (12 bytes)
            epc_bytes[0], epc_bytes[1], epc_bytes[2], epc_bytes[3],
            epc_bytes[4], epc_bytes[5], epc_bytes[6], epc_bytes[7],
            epc_bytes[8], epc_bytes[9], epc_bytes[10], epc_bytes[11],
            0x00,
            0x02,
            // New password bytes (4 bytes)
            new_password_bytes[0], new_password_bytes[1], new_password_bytes[2], new_password_bytes[3],
            // Old password bytes (4 bytes)
            old_password_bytes[0], old_password_bytes[1], old_password_bytes[2], old_password_bytes[3]};

        // Calculate CRC and send command
        crcValue = uiCrc16Cal(change_password_command, sizeof(change_password_command));
        crc1 = crcValue & 0xFF;
        crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(change_password_command, sizeof(change_password_command), crc1, crc2, true);
    }
};
