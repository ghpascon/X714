const String config_file = "/config.txt";

extern const int ant_qtd;
extern ANTENA_COMMANDS antena_commands;
extern ANTENA antena[];

extern byte session;
extern byte max_session;

extern bool read_on;
extern bool start_reading;
extern bool gpi_start;
extern int gpi_stop_delay;
extern bool always_send;

extern const byte min_power;
extern const byte max_power;
extern const byte min_rssi;
extern bool simple_send;
extern bool hotspot_on;
extern bool keyboard;
extern bool buzzer_on;
extern bool decode_gtin;

extern bool dhcp_on;
extern String static_ip;
extern String gateway_ip;
extern String subnet_mask;

extern bool webhook_on;
extern String webhook_url;

extern String prefix;

// ==================== Protected Inventory ====================
extern bool protected_inventory_enabled;
extern String protected_inventory_password;