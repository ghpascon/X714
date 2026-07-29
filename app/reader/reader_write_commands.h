class reader_write_commands : public commands_reader
{
public:
    // Write tag using EPC or TID with optional mask
    void write_tag(String new_epc, String password, String targetType, String mask_bytes = "")
    {
        if (targetType == "epc")
        {
            if (mask_bytes.length() == 0)
                write_tag_no_filter(new_epc, password); // sem EPC atual -> grava sem filtro
            else
                write_tag_epc(mask_bytes, new_epc, password);
        }
        else
        {
            write_tag_tid(new_epc, password, mask_bytes);
        }
    }

    // Write TID with mask
    // Grava EPC novo usando TID (12 bytes) como mascara de selecao (Write Data, ENum = 0xff).
    void write_tag_tid(String new_epc, String password, String mask_bytes)
    {
        if (!validateHex(mask_bytes, 24) || !validateHex(password, 8))
            return;
        if (new_epc.length() == 0 || new_epc.length() % 2 != 0 || !validateHex(new_epc, new_epc.length()))
            return;

        byte *new_bytes = to_bytes(new_epc);
        int new_len = new_epc.length() / 2;
        byte *pwd_bytes = to_bytes(password);
        const int pwd_len = 4;
        byte *tid_bytes = to_bytes(mask_bytes);
        const int tid_len = 12;

        int new_words = (new_len + 1) / 2; // palavras do EPC (arredondado pra cima)
        if (new_words < 1 || new_words > 31)
        {
            delete[] new_bytes;
            delete[] pwd_bytes;
            delete[] tid_bytes;
            return;
        }

        // O campo Wdt precisa ter EXATAMENTE new_words*2 bytes (unidade = palavra).
        // Se new_len for impar, completa com 1 byte de padding (0x00) no final.
        int new_padded_len = new_words * 2;
        int new_pad = new_padded_len - new_len; // 0 ou 1

        int wnum = new_words + 1; // PC + EPC, assim o PC tambem e reescrito

        // Campos do Data[] (Write Data, cmd 0x03), com Adr e Cmd inclusos na conta:
        // Adr(1) + Cmd(1) + WNum(1) + ENum(1) + Mem(1) + WordPtr(1)
        // + Wdt[ PC(2) + EPC(new_padded_len) ]
        // + Pwd(4) + MaskMem(1) + MaskAdr(2) + MaskLen(1) + MaskData(tid_len)
        int payload_size = 1 + 1 + 1 + 1 + 1 + 1                 // Adr,Cmd,WNum,ENum,Mem,WordPtr
                          + (2 + new_padded_len)                  // Wdt = PC + EPC (com padding)
                          + pwd_len                                // Pwd
                          + 1 + 2 + 1 + tid_len;                   // MaskMem,MaskAdr,MaskLen,MaskData
        int total_size = 1 + payload_size; // +1 para o byte de Len (indice 0)

        byte *reader_write_command = new byte[total_size];
        reader_write_command[1] = 0xff;       // Adr
        reader_write_command[2] = 0x03;       // Cmd = Write Data
        reader_write_command[3] = (byte)wnum; // WNum = PC + EPC
        reader_write_command[4] = 0xff;       // ENum = 0xff -> modo mascara (usa MaskMem/MaskAdr/MaskLen/MaskData)
        reader_write_command[5] = 0x01;       // Mem = EPC memory (onde vamos escrever)
        reader_write_command[6] = 0x01;       // WordPtr = 1 (comeca no PC), reescreve o PC junto

        int idx = 7;

        // Recalcula o PC para refletir o novo tamanho do EPC (evita "sobra" do EPC antigo)
        uint16_t pc_value = (uint16_t)((new_words & 0x1F) << 11);
        reader_write_command[idx++] = (byte)((pc_value >> 8) & 0xFF);
        reader_write_command[idx++] = (byte)(pc_value & 0xFF);

        for (int i = 0; i < new_len; ++i)
            reader_write_command[idx++] = new_bytes[i];
        for (int i = 0; i < new_pad; ++i)
            reader_write_command[idx++] = 0x00; // padding pra completar palavra, se necessario

        for (int i = 0; i < pwd_len; ++i)
            reader_write_command[idx++] = pwd_bytes[i];

        reader_write_command[idx++] = 0x02; // MaskMem = TID
        reader_write_command[idx++] = 0x00; // MaskAdr high
        reader_write_command[idx++] = 0x00; // MaskAdr low  (bit 0 do banco TID)
        reader_write_command[idx++] = 0x60; // MaskLen = 96 bits (12 bytes)

        for (int i = 0; i < tid_len; ++i)
            reader_write_command[idx++] = tid_bytes[i];

        reader_write_command[0] = (byte)(payload_size + 2); // Len = bytes apos o Len + CRC(2)

        uint16_t crcValue = uiCrc16Cal(reader_write_command, total_size);
        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(reader_write_command, total_size, crc1, crc2);

        delete[] reader_write_command;
        delete[] new_bytes;
        delete[] pwd_bytes;
        delete[] tid_bytes;
    }

    // Write EPC (filtrando pelo EPC atual)
    void write_tag_epc(String current_epc, String new_epc, String password)
    {
        // current_epc e o EPC de selecao (ENum). Pelo doc, ENum deve ser
        // 0..15 palavras (max 30 hex chars), e nao pode ser vazio
        // (vazio = filtro invalido, trava/reinicia o leitor).
        if (current_epc.length() == 0 || current_epc.length() % 2 != 0 || current_epc.length() > 30)
            return;
        if (!validateHex(current_epc, current_epc.length()) || !validateHex(new_epc, new_epc.length()) || !validateHex(password, 8))
            return;
        if (new_epc.length() == 0 || new_epc.length() % 2 != 0)
            return;

        byte *current_bytes = to_bytes(current_epc);
        int current_len = current_epc.length() / 2;
        byte *new_bytes = to_bytes(new_epc);
        int new_len = new_epc.length() / 2;
        byte *pwd_bytes = to_bytes(password);
        const int pwd_len = 4;

        int new_words = (new_len + 1) / 2;
        if (new_words < 1 || new_words > 31)
        {
            delete[] current_bytes;
            delete[] new_bytes;
            delete[] pwd_bytes;
            return;
        }
        int current_words = (current_len + 1) / 2;
        if (current_words > 15) // ENum maximo, conforme o doc
        {
            delete[] current_bytes;
            delete[] new_bytes;
            delete[] pwd_bytes;
            return;
        }

        // Tanto o campo EPC de selecao quanto o Wdt precisam ter numero par de bytes
        // (unidade = palavra). Completa com 0x00 se algum dos dois for impar.
        int current_padded_len = current_words * 2;
        int current_pad = current_padded_len - current_len; // 0 ou 1
        int new_padded_len = new_words * 2;
        int new_pad = new_padded_len - new_len; // 0 ou 1

        int wnum = new_words + 1; // PC + EPC

        // Campos do Data[] com Adr e Cmd inclusos na conta:
        // Adr(1) + Cmd(1) + WNum(1) + ENum(1) + EPC(current_padded_len)
        // + Mem(1) + WordPtr(1) + Wdt[ PC(2) + EPC(new_padded_len) ] + Pwd(4)
        int payload_size = 1 + 1 + 1 + 1                          // Adr,Cmd,WNum,ENum
                          + current_padded_len                     // EPC de selecao (com padding)
                          + 1 + 1                                  // Mem, WordPtr
                          + (2 + new_padded_len)                    // Wdt = PC + EPC novo (com padding)
                          + pwd_len;                                // Pwd
        int total_size = 1 + payload_size;

        byte *reader_write_command = new byte[total_size];
        reader_write_command[1] = 0xff; // Adr
        reader_write_command[2] = 0x03; // Cmd = Write Data
        reader_write_command[3] = (byte)wnum;
        reader_write_command[4] = (byte)current_words;

        int idx = 5;
        for (int i = 0; i < current_len; ++i)
            reader_write_command[idx++] = current_bytes[i];
        for (int i = 0; i < current_pad; ++i)
            reader_write_command[idx++] = 0x00;

        reader_write_command[idx++] = 0x01; // Mem = EPC
        reader_write_command[idx++] = 0x01; // WordPtr = 1 (PC)

        uint16_t pc_value = (uint16_t)((new_words & 0x1F) << 11);
        reader_write_command[idx++] = (byte)((pc_value >> 8) & 0xFF);
        reader_write_command[idx++] = (byte)(pc_value & 0xFF);

        for (int i = 0; i < new_len; ++i)
            reader_write_command[idx++] = new_bytes[i];
        for (int i = 0; i < new_pad; ++i)
            reader_write_command[idx++] = 0x00;

        for (int i = 0; i < pwd_len; ++i)
            reader_write_command[idx++] = pwd_bytes[i];

        reader_write_command[0] = (byte)(payload_size + 2);

        uint16_t crcValue = uiCrc16Cal(reader_write_command, total_size);
        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(reader_write_command, total_size, crc1, crc2);

        delete[] reader_write_command;
        delete[] current_bytes;
        delete[] new_bytes;
        delete[] pwd_bytes;
    }

    // Write EPC without any filter
    void write_tag_no_filter(String new_epc, String password)
    {
        if (!validateHex(new_epc, new_epc.length()) || !validateHex(password, 8))
            return;

        byte *epc_bytes = to_bytes(new_epc);
        int epc_len = new_epc.length() / 2; // bytes

        byte *pwd_bytes = to_bytes(password);
        int pwd_len = 4;

        int words = (epc_len + 1) / 2; // number of 16-bit words (ceil)

        // payload (everything after the length byte): ff, 04, words, password(4), epc(epc_len)
        int payload_size = 1 + 1 + 1 + pwd_len + epc_len;
        int total_size = 1 + payload_size; // include length byte

        byte *reader_write_command = new byte[total_size];

        // fill payload starting at index 1
        reader_write_command[1] = 0xff;
        reader_write_command[2] = 0x04;
        reader_write_command[3] = (byte)words;

        int idx = 4;
        for (int i = 0; i < pwd_len; ++i)
            reader_write_command[idx++] = pwd_bytes[i];

        for (int i = 0; i < epc_len; ++i)
            reader_write_command[idx++] = epc_bytes[i];

        // length field follows existing project convention: payload_size + 2
        reader_write_command[0] = (byte)(payload_size + 2);

        uint16_t crcValue = uiCrc16Cal(reader_write_command, total_size);
        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(reader_write_command, total_size, crc1, crc2);

        delete[] reader_write_command;
        delete[] epc_bytes;
        delete[] pwd_bytes;
    }

    // Change password command parser
    void change_password(String epc, String new_password, String old_password = "00000000")
    {
        // Validate input parameters
        if (epc.length() != 24 || new_password.length() != 8 || old_password.length() != 8)
            return; // Invalid parameters

        // Validate hex strings
        if (!validateHex(epc, 24) || !validateHex(new_password, 8) || !validateHex(old_password, 8))
            return; // Invalid hex format

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
        uint16_t crcValue = uiCrc16Cal(change_password_command, sizeof(change_password_command));
        byte crc1 = crcValue & 0xFF;
        byte crc2 = (crcValue >> 8) & 0xFF;
        write_bytes(change_password_command, sizeof(change_password_command), crc1, crc2, true);
    }
};