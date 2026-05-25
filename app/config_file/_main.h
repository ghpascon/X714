#include "vars.h"

// Arquivos usados na escrita atômica
static const String config_file_tmp = "/config.txt.tmp";
static const String config_file_bak = "/config.txt.bak";

class CONFIG_FILE
{
private:
    String _buf; // buffer reutilizado para montar config

    // ------------------------------------------------------------------
    // Helpers de I/O
    // ------------------------------------------------------------------

    // Copia src → dst; retorna true se OK
    static bool _copyFile(const String &src, const String &dst)
    {
        File in = LittleFS.open(src, "r");
        if (!in) return false;

        File out = LittleFS.open(dst, "w");
        if (!out) { in.close(); return false; }

        uint8_t chunk[64];
        while (in.available())
        {
            size_t n = in.read(chunk, sizeof(chunk));
            if (n) out.write(chunk, n);
        }
        in.close();
        out.close();
        return true;
    }

    // Retorna true se o arquivo existe e tem tamanho > 0
    static bool _fileValid(const String &path)
    {
        File f = LittleFS.open(path, "r");
        if (!f) return false;
        size_t sz = f.size();
        f.close();
        return sz > 0;
    }

    // ------------------------------------------------------------------
    // Parse / apply
    // ------------------------------------------------------------------

    static bool _parseBool(String v)
    {
        v.trim();
        v.toLowerCase();
        return (v == "on" || v == "1" || v == "true" || v == "yes");
    }

    void _applyParam(const String &key, String value)
    {
        // Antena: <num>,<on|off>,<power>,<rssi>
        if (key == "antena")
        {
            int s1 = value.indexOf(',');                if (s1 < 0) return;
            int s2 = value.indexOf(',', s1 + 1);        if (s2 < 0) return;
            int s3 = value.indexOf(',', s2 + 1);        if (s3 < 0) return;

            String t1 = value.substring(0, s1);        t1.trim();
            String t2 = value.substring(s1 + 1, s2);   t2.trim();
            String t3 = value.substring(s2 + 1, s3);   t3.trim();
            String t4 = value.substring(s3 + 1);       t4.trim();

            int idx   = (int)t1.toInt();
            bool act  = _parseBool(t2);
            int pwr   = constrain((int)t3.toInt(), (int)min_power, (int)min((byte)27, max_power));
            int rssi  = max((int)t4.toInt(), (int)min_rssi);

            if (idx < 1 || idx > ant_qtd) return;
            antena_commands.set_antena(idx, act, (byte)pwr, (byte)rssi);
            return;
        }

        if (key == "session")               { int v = value.toInt(); session = (v < 0 || v > max_session) ? 0 : v; return; }
        if (key == "start_reading")         { start_reading = _parseBool(value); read_on = start_reading; return; }
        if (key == "gpi_start")             { gpi_start       = _parseBool(value); return; }
        if (key == "gpi_stop_delay")        { int v = value.toInt(); gpi_stop_delay = max(v, 0); return; }
        if (key == "always_send")           { always_send     = _parseBool(value); return; }
        if (key == "simple_send")           { simple_send     = _parseBool(value); return; }
        if (key == "hotspot_on")            { hotspot_on      = _parseBool(value); return; }
        if (key == "keyboard")              { keyboard        = _parseBool(value); return; }
        if (key == "buzzer_on")             { buzzer_on       = _parseBool(value); return; }
        if (key == "decode_gtin")           { decode_gtin     = _parseBool(value); return; }
        if (key == "dhcp_on")               { dhcp_on         = _parseBool(value); return; }
        if (key == "static_ip")             { static_ip       = value; return; }
        if (key == "gateway_ip")            { gateway_ip      = value; return; }
        if (key == "subnet_mask")           { subnet_mask     = value; return; }
        if (key == "webhook_on")            { webhook_on      = _parseBool(value); return; }
        if (key == "webhook_url")           { webhook_url     = value; return; }
        if (key == "prefix")                { value.replace(" ", ","); prefix = value; return; }
        if (key == "protected_inventory_enabled")  { protected_inventory_enabled  = _parseBool(value); return; }
        if (key == "protected_inventory_password") { protected_inventory_password = value; return; }
    }

    // ------------------------------------------------------------------
    // Montagem do buffer
    // ------------------------------------------------------------------

    void _buildConfig()
    {
        _buf = "";

        for (int i = 0; i < ant_qtd; i++)
        {
            _buf += "antena:";
            _buf += String(i + 1);        _buf += ',';
            _buf += antena[i].active ? "on" : "off"; _buf += ',';
            _buf += String(antena[i].power);          _buf += ',';
            _buf += String(antena[i].rssi);           _buf += '\n';
        }

        _buf += "session:"            + String(session)                                          + "\n";
        _buf += "start_reading:"      + String(start_reading     ? "on" : "off")                 + "\n";
        _buf += "gpi_start:"          + String(gpi_start         ? "on" : "off")                 + "\n";
        _buf += "gpi_stop_delay:"     + String(gpi_stop_delay)                                   + "\n";
        _buf += "always_send:"        + String(always_send       ? "on" : "off")                 + "\n";
        _buf += "simple_send:"        + String(simple_send       ? "on" : "off")                 + "\n";
        _buf += "hotspot_on:"         + String(hotspot_on        ? "on" : "off")                 + "\n";
        _buf += "keyboard:"           + String(keyboard          ? "on" : "off")                 + "\n";
        _buf += "buzzer_on:"          + String(buzzer_on         ? "on" : "off")                 + "\n";
        _buf += "decode_gtin:"        + String(decode_gtin       ? "on" : "off")                 + "\n";
        _buf += "dhcp_on:"            + String(dhcp_on           ? "on" : "off")                 + "\n";
        _buf += "static_ip:"          + static_ip                                                + "\n";
        _buf += "gateway_ip:"         + gateway_ip                                               + "\n";
        _buf += "subnet_mask:"        + subnet_mask                                              + "\n";
        _buf += "webhook_on:"         + String(webhook_on        ? "on" : "off")                 + "\n";
        _buf += "webhook_url:"        + webhook_url                                              + "\n";
        _buf += "prefix:"             + prefix                                                   + "\n";
        _buf += "protected_inventory_enabled:"  + String(protected_inventory_enabled  ? "on" : "off") + "\n";
        _buf += "protected_inventory_password:" + protected_inventory_password                   + "\n";
    }

    // Grava _buf em path; retorna true se OK
    bool _writeBuf(const String &path)
    {
        File f = LittleFS.open(path, "w");
        if (!f) return false;
        f.print(_buf);
        f.close();
        return true;
    }

    // Escrita atômica: tmp → bak → config
    // Mesmo que a energia caia no meio, bak ou config ainda tem conteúdo válido
    bool _atomicSave()
    {
        // 1. Grava conteúdo novo em .tmp
        if (!_writeBuf(config_file_tmp))
        {
            LittleFS.remove(config_file_tmp);
            return false;
        }

        // 2. Promove config atual → bak (melhor esforço; ignora falha)
        if (_fileValid(config_file))
            _copyFile(config_file, config_file_bak);

        // 3. Promove .tmp → config
        if (!_copyFile(config_file_tmp, config_file))
        {
            LittleFS.remove(config_file_tmp);
            return false;
        }

        LittleFS.remove(config_file_tmp);
        return true;
    }

    // Lê e aplica todas as linhas de um arquivo aberto
    void _parseFile(File &f)
    {
        while (f.available())
        {
            String line = f.readStringUntil('\n');
            line.replace("\r", "");
            line.trim();
            if (line.length() == 0) continue;

            int sep = line.indexOf(':');
            if (sep < 0) continue;

            String key = line.substring(0, sep);
            String val = line.substring(sep + 1);
            key.trim();
            key.toLowerCase();
            val.trim();

            _applyParam(key, val);
        }
    }

public:
    // ------------------------------------------------------------------
    // API pública
    // ------------------------------------------------------------------

    // Chame no setup(): carrega config principal; se inválida tenta o backup
    void get_config()
    {
        // Reserva buffer uma vez para aliviar alocações no save
        if (_buf.length() == 0)
            _buf.reserve((size_t)ant_qtd * 32u + 512u);

        const String *source = nullptr;

        if (_fileValid(config_file))
            source = &config_file;
        else if (_fileValid(config_file_bak))
        {
            Serial.println("[CFG] config.txt ausente/vazio, usando backup");
            source = &config_file_bak;
        }
        else
        {
            Serial.println("[CFG] Nenhum arquivo de config encontrado, usando padroes");
            return;
        }

        File f = LittleFS.open(*source, "r");
        if (!f)
        {
            Serial.println("[CFG] Falha ao abrir arquivo de config");
            return;
        }
        _parseFile(f);
        f.close();
    }

    // Chame no loop(): salva apenas se a config mudou e o intervalo passou
    // Retorna true se efetivamente gravou no flash
    bool save_config()
    {
        static const unsigned long SAVE_INTERVAL_MS = 10000UL;
        static unsigned long last_save_ms = 0;
        static String        last_saved   = "";
        static bool          initialized  = false;

        // Throttle: nunca salva mais de 1x a cada SAVE_INTERVAL_MS
        if ((millis() - last_save_ms) < SAVE_INTERVAL_MS)
            return false;

        last_save_ms = millis();
        _buildConfig();

        // Na primeira passagem pelo loop apenas inicializa o cache
        // sem gravar (a config acabou de ser lida do disco no setup)
        if (!initialized)
        {
            last_saved  = _buf;
            initialized = true;
            return false;
        }

        if (_buf == last_saved)
            return false; // nada mudou

        if (!_atomicSave())
        {
            Serial.println("[CFG] Erro ao salvar config");
            return false;
        }

        last_saved = _buf;
        Serial.println("[CFG] Config salva");
        return true;
    }

    // Força gravação imediata ignorando throttle e cache (use em shutdown/reset planejado)
    // Retorna true se gravou com sucesso
    bool force_save()
    {
        _buildConfig();
        if (!_atomicSave())
        {
            Serial.println("[CFG] Erro ao forçar save");
            return false;
        }
        Serial.println("[CFG] Config salva (forcado)");
        return true;
    }
};