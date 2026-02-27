// FreeRTOS semaphore type
#include <freertos/semphr.h>

// ==================== Antena ====================
ANTENA antena[ant_qtd];
ANTENA_COMMANDS antena_commands;

// ==================== Tags ====================
TAG tags[max_tags];
int current_tag = 0;
TAG_COMMANDS tag_commands;

// ==================== Serial ====================
CONNECTION connection;
MySerial myserial;
MySerialCheck myserialcheck;

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

// ==================== Modos ====================

// ==================== Globais gerais ====================

bool fs_loaded = true;

// ==================== Watchdog ====================

bool btConnected = false;
bool eth_connected = false;

// ==================== Ethernet Configuration ====================

// WEBHOOK

// ======= prefix =======

// ==================== Protected Inventory ====================

bool reader_in_protected_inventory = false;