class protected_mode : public commands_reader
{
public:
    void protected_mode_tag(String epc, String password, bool enable)
    {
        // Validate input parameters: accept EPC shorter than 24 hex chars
        if (epc.length() == 0 || epc.length() % 2 != 0 || epc.length() > 24 || password.length() != 8)
        {
            return; // Invalid parameters
        }
        if (!validateHex(epc, epc.length()) || !validateHex(password, 8))
        {
            return; // Invalid hex format
        }

        int epc_len = epc.length() / 2; // bytes reais do EPC informado
        byte *epc_bytes = to_bytes(epc);

        // Numero de palavras (16 bits) necessarias, arredondando pra cima
        int epc_words = (epc_len + 1) / 2;
        int epc_padded_len = epc_words * 2; // bytes que de fato vao no comando
        int epc_pad = epc_padded_len - epc_len; // 0 ou 1

        // Convert password hex string to bytes (4 bytes from 8 hex chars)
        byte password_bytes[4];
        for (int i = 0; i < 4; i++)
        {
            String byteStr = password.substring(i * 2, i * 2 + 2);
            password_bytes[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
        }

        // Build command dynamically (match write_* pattern)
        // Adr(1) + Cmd(1) + Words(1) + EPC(epc_padded_len) + Enable(1) + Pwd(4)
        int payload_size = 1 + 1 + 1 + epc_padded_len + 1 + 4;
        int total_size = 1 + payload_size; // +1 for Len byte

        byte *reader_protected_mode_command = new byte[total_size];
        int idx = 1;
        reader_protected_mode_command[idx++] = 0xff;         // Adr
        reader_protected_mode_command[idx++] = 0xe9;         // Cmd
        reader_protected_mode_command[idx++] = (byte)epc_words; // palavras reais do EPC, sem arredondamento errado

        for (int i = 0; i < epc_len; ++i)
            reader_protected_mode_command[idx++] = epc_bytes[i];
        for (int i = 0; i < epc_pad; ++i)
            reader_protected_mode_command[idx++] = 0x00; // padding so se sobrar byte impar

        reader_protected_mode_command[idx++] = enable ? 0x01 : 0x00;

        for (int i = 0; i < 4; i++)
            reader_protected_mode_command[idx++] = password_bytes[i];

        reader_protected_mode_command[0] = (byte)(payload_size + 2);

        uint16_t crcValue = uiCrc16Cal(reader_protected_mode_command, total_size);
        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(reader_protected_mode_command, total_size, crc1, crc2, false);

        delete[] reader_protected_mode_command;
        delete[] epc_bytes;
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

        uint16_t crcValue = uiCrc16Cal(
            protected_inventory_command,
            sizeof(protected_inventory_command));

        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;

        write_bytes(
            protected_inventory_command,
            sizeof(protected_inventory_command),
            crc1,
            crc2,
            false);

        reader_in_protected_inventory = enable;
    }
};