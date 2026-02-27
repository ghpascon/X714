// FreeRTOS semaphore type
#include <freertos/semphr.h>

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

// ==================== LEDs RGB ====================
LED_RGB rgb;

// ==================== Leitura ====================
bool read_on = false;

bool fs_loaded = true;

bool btConnected = false;
bool eth_connected = false;

bool reader_in_protected_inventory = false;