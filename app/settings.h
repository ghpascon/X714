class Settings
{
public:
    Preferences prefs;
    unsigned long lastSaveMillis = 0;
    String mirrorValues = "";
    // READING
    const byte ant_qtd = 4;
    const byte min_power = 10;
    const byte max_power = 27;
    const byte min_rssi = 40;
    const int max_tags = 300;
    byte max_session = 0x03;

    String prefix = "";
    byte current_session = 0x01;
    byte current_rssi = 150;

    // MODES
    bool simple_send = false;
    bool keyboard = false;
    bool start_reading = false;
    bool gpi_start = false;
    bool always_send = false;
    bool hotspot_on = true;
    bool buzzer_on = false;
    bool decode_gtin = true;

    // ETHERNET
    bool dhcp_on = false;
    String static_ip = "192.168.1.101";
    String gateway_ip = "192.168.1.1";
    String subnet_mask = "255.255.255.0";

    // WEBHOOK
    bool webhook_on = false;
    String webhook_url = "http://192.168.1.10:5001";

    // PROTECTED INVENTORY
    bool protected_inventory_enabled = false;
    String protected_inventory_password = "00000000";

    // ANTENAS
    String ant_cfg = "1:1-22;2:0-22;3:0-22;4:0-22";

    void set_rssi_filter(byte value)
    {
        rssi_filter = max(value, current_rssi);
    }

    void set_session(byte value)
    {
        current_session = constrain(value, 0, max_session);
    }

    void set_ant_cfg(String cfg)
    {
        ant_cfg = cfg;
    }

    void load()
    {
        prefs.begin("settings", true);
        prefix = prefs.getString("prefix", "");
        current_session = prefs.getUChar("current_session", 0x01);
        current_rssi = prefs.getUChar("current_rssi", 150);
        simple_send = prefs.getBool("simple_send", false);
        keyboard = prefs.getBool("keyboard", false);
        start_reading = prefs.getBool("start_reading", false);
        gpi_start = prefs.getBool("gpi_start", false);
        always_send = prefs.getBool("always_send", false);
        hotspot_on = prefs.getBool("hotspot_on", true);
        buzzer_on = prefs.getBool("buzzer_on", false);
        decode_gtin = prefs.getBool("decode_gtin", true);
        dhcp_on = prefs.getBool("dhcp_on", false);
        static_ip = prefs.getString("static_ip", "192.168.1.101");
        gateway_ip = prefs.getString("gateway_ip", "192.168.1.1");
        subnet_mask = prefs.getString("subnet_mask", "255.255.255.0");
        webhook_on = prefs.getBool("webhook_on", false);
        webhook_url = prefs.getString("webhook_url", "http://192.168.1.10:5001");
        protected_inventory_enabled = prefs.getBool("protected_inventory_enabled", false);
        protected_inventory_password = prefs.getString("protected_inventory_password", "00000000");
        ant_cfg = prefs.getString("ant_cfg", "1:1-22;2:0-22;3:0-22;4:0-22");
        prefs.end();

        mirrorValues = getMirrorString();
    }

    void save()
    {
        unsigned long now = millis();
        if (now - lastSaveMillis < 5000)
            return;
        lastSaveMillis = now;

        String currentMirror = getMirrorString();
        if (currentMirror != mirrorValues)
        {
            prefs.begin("settings", false);
            prefs.putString("prefix", prefix);
            prefs.putUChar("current_session", current_session);
            prefs.putUChar("current_rssi", current_rssi);
            prefs.putBool("simple_send", simple_send);
            prefs.putBool("keyboard", keyboard);
            prefs.putBool("start_reading", start_reading);
            prefs.putBool("gpi_start", gpi_start);
            prefs.putBool("always_send", always_send);
            prefs.putBool("hotspot_on", hotspot_on);
            prefs.putBool("buzzer_on", buzzer_on);
            prefs.putBool("decode_gtin", decode_gtin);
            prefs.putBool("dhcp_on", dhcp_on);
            prefs.putString("static_ip", static_ip);
            prefs.putString("gateway_ip", gateway_ip);
            prefs.putString("subnet_mask", subnet_mask);
            prefs.putBool("webhook_on", webhook_on);
            prefs.putString("webhook_url", webhook_url);
            prefs.putBool("protected_inventory_enabled", protected_inventory_enabled);
            prefs.putString("protected_inventory_password", protected_inventory_password);
            prefs.putString("ant_cfg", ant_cfg);
            prefs.end();
            mirrorValues = currentMirror;
        }
    }

    String getMirrorString()
    {
        String s = "";
        s += prefix + ",";
        s += String(current_session) + ",";
        s += String(current_rssi) + ",";
        s += String(simple_send) + ",";
        s += String(keyboard) + ",";
        s += String(start_reading) + ",";
        s += String(gpi_start) + ",";
        s += String(always_send) + ",";
        s += String(hotspot_on) + ",";
        s += String(buzzer_on) + ",";
        s += String(decode_gtin) + ",";
        s += String(dhcp_on) + ",";
        s += static_ip + ",";
        s += gateway_ip + ",";
        s += subnet_mask + ",";
        s += String(webhook_on) + ",";
        s += webhook_url + ",";
        s += String(protected_inventory_enabled) + ",";
        s += protected_inventory_password + ",";
        s += ant_cfg;
        return s;
    }
};