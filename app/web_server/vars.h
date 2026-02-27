#include <WebServer.h>
WebServer server(80);

#include <Update.h>

extern READER reader_module;

extern const int ant_qtd;
extern ANTENA antena[];
extern ANTENA_COMMANDS antena_commands;

extern const byte min_power;
extern const byte max_power;
extern const byte min_rssi;

extern TAG_COMMANDS tag_commands;
extern const int max_tags;
extern TAG tags[];
extern int current_tag;

extern bool read_on;
extern byte temperatura;

extern byte session;
extern byte max_session;

extern CONFIG_FILE config_file_commands;

extern bool start_reading;
extern bool gpi_start;
extern int gpi_stop_delay;
extern bool always_send;

extern const int gpo_qtd;
extern bool gpo[];

extern bool simple_send;

extern bool hotspot_on;
extern bool keyboard;
extern bool buzzer_on;
extern bool decode_gtin;

extern bool dhcp_on;
extern String static_ip;
extern String gateway_ip;
extern String subnet_mask;

// Webhook
extern bool webhook_on;
extern String webhook_url;

// Prefix
extern String prefix;

// Protected Inventory
extern bool protected_inventory_enabled;
extern String protected_inventory_password;