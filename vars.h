// FreeRTOS semaphore type
#include <freertos/semphr.h>

// ==================== Antena ====================
const int ant_qtd = 4;
ANTENA antena[ant_qtd];
ANTENA_COMMANDS antena_commands;
const byte min_power = 1;
const byte max_power = 27;
const byte min_rssi = 40;

// ==================== Tags ====================
const int max_tags = 300;
TAG tags[max_tags];
int current_tag = 0;
TAG_COMMANDS tag_commands;

// ==================== Serial ====================
CONNECTION connection;
MySerial myserial;
MySerialCheck myserialcheck;
bool simple_send = false;
bool keyboard = false;

// ==================== Pinos ====================
PINS pins;

// ==================== Leitor ====================
READER reader_module;

// ==================== Servidor Web ====================
WEB_SERVER web_server;
WEBHOOK webhook;

// ==================== Configurações ====================
CONFIG_FILE config_file_commands;

// ==================== LEDs RGB ====================
LED_RGB rgb;

// ==================== Leitura ====================
bool read_on = false;
byte session = 0x00;
byte max_session = 0x03;

// ==================== Modos ====================
bool start_reading = false;
bool gpi_start = false;
bool always_send = false;

// ==================== Globais gerais ====================
byte temperatura = 0;
int gpi_stop_delay = 0;
bool hotspot_on = true;
bool buzzer_on = false;
bool decode_gtin = true;
bool fs_loaded = true;

// ==================== Watchdog ====================
#define WATCHDOG_TIMEOUT 60

bool btConnected = false;
bool eth_connected = false;
String eth_state = "";

// ==================== Ethernet Configuration ====================
bool dhcp_on = false;
String static_ip = "192.168.1.101";
String gateway_ip = "192.168.1.1";
String subnet_mask = "255.255.255.0";

// WEBHOOK
bool webhook_on = false;
String webhook_url = "http://192.168.1.10:5001";

// ======= prefix =======
String prefix = "";

// ==================== Protected Inventory ====================
bool protected_inventory_enabled = false;
String protected_inventory_password = "00000000";
bool reader_in_protected_inventory = false;

