class TELNET
{
protected:
    WiFiServer telnetServer{23};
    WiFiClient telnetClient;
    bool telnetServerStarted = false;
    String telnetRxBuffer;
    unsigned long telnetLastByteMs = 0;
    static const size_t MAX_TELNET_LINE_LEN = 256;
    static const uint32_t TELNET_STALE_INPUT_MS = 3000;
    // Parser states for TELNET control negotiation (IAC sequences).
    uint8_t telnetParseState = 0;

private:
    enum : uint8_t
    {
        PARSE_DATA = 0,
        PARSE_IAC,
        PARSE_IAC_OPTION,
        PARSE_IAC_SB_OPTION,
        PARSE_IAC_SB_DATA,
        PARSE_IAC_SB_IAC
    };

    void reset_telnet_rx_state()
    {
        telnetRxBuffer = "";
        telnetLastByteMs = 0;
        telnetParseState = PARSE_DATA;
    }

    bool consume_telnet_meta(uint8_t c)
    {
        switch (telnetParseState)
        {
        case PARSE_DATA:
            if (c == 0xFF)
            {
                telnetParseState = PARSE_IAC;
                return true;
            }
            return false;

        case PARSE_IAC:
            if (c == 0xFF)
            {
                // Escaped 0xFF data byte.
                telnetParseState = PARSE_DATA;
                return false;
            }

            if (c == 0xFA)
            {
                // Subnegotiation begins: IAC SB <option> ... IAC SE
                telnetParseState = PARSE_IAC_SB_OPTION;
                return true;
            }

            if (c == 0xFB || c == 0xFC || c == 0xFD || c == 0xFE)
            {
                // WILL/WONT/DO/DONT + option byte
                telnetParseState = PARSE_IAC_OPTION;
                return true;
            }

            // One-byte IAC command.
            telnetParseState = PARSE_DATA;
            return true;

        case PARSE_IAC_OPTION:
            telnetParseState = PARSE_DATA;
            return true;

        case PARSE_IAC_SB_OPTION:
            telnetParseState = PARSE_IAC_SB_DATA;
            return true;

        case PARSE_IAC_SB_DATA:
            if (c == 0xFF)
                telnetParseState = PARSE_IAC_SB_IAC;
            return true;

        case PARSE_IAC_SB_IAC:
            if (c == 0xF0)
            {
                // End subnegotiation (IAC SE)
                telnetParseState = PARSE_DATA;
            }
            else if (c == 0xFF)
            {
                // Escaped 0xFF inside subnegotiation payload.
                telnetParseState = PARSE_IAC_SB_DATA;
            }
            else
            {
                // Keep skipping until IAC SE.
                telnetParseState = PARSE_IAC_SB_DATA;
            }
            return true;

        default:
            telnetParseState = PARSE_DATA;
            return true;
        }
    }

    String finalize_telnet_line()
    {
        String line = telnetRxBuffer;
        telnetRxBuffer = "";
        telnetLastByteMs = 0;
        line.trim();
        return line;
    }

public:
    void config_telnet()
    {
        if (telnetServerStarted)
            return;

        telnetServer.begin();
        telnetServer.setNoDelay(true);
        telnetServerStarted = true;
        telnetRxBuffer.reserve(MAX_TELNET_LINE_LEN);
    }

    // Aceita novo cliente se necessário
    void maintain_client()
    {
        if (!telnetServerStarted)
            return;

        // Drena a fila de conexoes pendentes, mantendo no maximo 1 cliente ativo.
        while (true)
        {
            WiFiClient pendingClient = telnetServer.available();
            if (!pendingClient)
                break;

            if (telnetClient && telnetClient.connected())
            {
                pendingClient.println("#BUSY:ONLY_ONE_CLIENT_ALLOWED");
                pendingClient.println("#DISCONNECTING");
                pendingClient.println("#CONNECTED_CLIENT:" + String(telnetClient.remoteIP().toString()));
                pendingClient.stop();
            }
            else
            {
                if (telnetClient)
                    telnetClient.stop();
                telnetClient = pendingClient;
                telnetClient.setNoDelay(true);
                telnetClient.setTimeout(20);
                reset_telnet_rx_state();
                telnetClient.println("#CONNECTED");
            }
        }

        if (telnetClient && !telnetClient.connected())
        {
            telnetClient.stop();
            reset_telnet_rx_state();
        }
    }

    bool telnet_write(const String &msg, bool newline = true)
    {
        maintain_client();
        if (!telnetClient || !telnetClient.connected())
            return false;

        if (newline)
            return telnetClient.println(msg) > 0;
        return telnetClient.print(msg) > 0;
    }

    // Verifica se chegou algo no telnet; lê até '\n' ou até timeout_ms e faz um print no Serial
    String check_telnet(uint32_t timeout_ms = 1200)
    {
        maintain_client();

        if (!telnetClient || !telnetClient.connected())
        {
            reset_telnet_rx_state();
            return "";
        }

        while (telnetClient.available())
        {
            int raw = telnetClient.read();
            if (raw < 0)
                break;

            uint8_t c = (uint8_t)raw;
            if (consume_telnet_meta(c))
                continue;

            if (c == '\b' || c == 0x7F)
            {
                if (telnetRxBuffer.length() > 0)
                    telnetRxBuffer.remove(telnetRxBuffer.length() - 1);
                continue;
            }

            if (c == '\r' || c == '\n')
            {
                String line = finalize_telnet_line();
                if (line.length() > 0)
                    return line;
                continue;
            }

            if (c == '\0')
                continue;

            if (c == '\t')
                c = ' ';

            if (c < 32 || c > 126)
                continue;

            if (telnetRxBuffer.length() >= MAX_TELNET_LINE_LEN)
            {
                // Descarta linha malformada/excessiva sem executar comando parcial.
                telnetRxBuffer = "";
                telnetClient.println("#ERROR:CMD_TOO_LONG");
                telnetLastByteMs = 0;
                continue;
            }

            telnetRxBuffer += (char)c;
            telnetLastByteMs = millis();
        }

        if (telnetRxBuffer.length() > 0 && telnetLastByteMs > 0)
        {
            const uint32_t staleLimit = timeout_ms > 0 ? timeout_ms : TELNET_STALE_INPUT_MS;
            if (millis() - telnetLastByteMs >= staleLimit)
            {
                // Nao retorna comando parcial por timeout; apenas limpa o buffer.
                telnetRxBuffer = "";
                telnetLastByteMs = 0;
            }
        }

        return "";
    }

    void telnet_loop()
    {
        maintain_client();

        // Limpa lixo de entrada antiga que nao terminou em newline.
        if (telnetRxBuffer.length() > 0 && telnetLastByteMs > 0)
        {
            if (millis() - telnetLastByteMs >= TELNET_STALE_INPUT_MS)
            {
                telnetRxBuffer = "";
                telnetLastByteMs = 0;
            }
        }
    }
};
