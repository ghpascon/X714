class Settings
{
public:
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

    void set_rssi_filter(byte value)
    {
        rssi_filter = max(value, current_rssi);
    }

    void set_session(byte value)
    {
        current_session = constrain(value, 0, max_session);
    }
};