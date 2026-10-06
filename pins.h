#ifndef PINS_H
#define PINS_H

// Model options:
#define ESP_MODEL_ESP32_S3 1
#define ESP_MODEL_ESP32_S3_ETH 2

// Default model when not explicitly defined.
#ifndef ESP_MODEL
#define ESP_MODEL ESP_MODEL_ESP32_S3
#endif

// #define INVERT_RGB

#if (ESP_MODEL == ESP_MODEL_ESP32_S3)
#define ETH_MISO_PIN 21
#define ETH_MOSI_PIN 35
#define ETH_SCLK_PIN 45
#define ETH_CS_PIN 47
#define ETH_INT_PIN -1
#define ETH_RST_PIN -1
#define ETH_ADDR 1

#define in_1_pin 18
#define in_2_pin 11
#define in_3_pin 12

#define buzzer_pin 4
#define indicator_pin 42
#define out_1_pin 41
#define out_2_pin 40
#define out_3_pin 39

#define tx_reader_module 13
#define rx_reader_module 14

#define RGB_DATA_PIN 48
#define EXTERNAL_LED_RED_PIN 5
#define EXTERNAL_LED_GREEN_PIN 6
#define EXTERNAL_LED_BLUE_PIN 7

#define POWER_PIN 8

const byte LED_ANT_PINS[ant_qtd] = {15, 16, 17, 1};

#define TEST_PIN 0

#elif (ESP_MODEL == ESP_MODEL_ESP32_S3_ETH)
#define ETH_MISO_PIN 11
#define ETH_MOSI_PIN 12
#define ETH_SCLK_PIN 10
#define ETH_CS_PIN 9
#define ETH_INT_PIN 13
#define ETH_RST_PIN 14
#define ETH_ADDR 1

#define in_1_pin 8
#define in_2_pin 19
#define in_3_pin 20

#define buzzer_pin 25
#define indicator_pin 15
#define out_1_pin 16
#define out_2_pin 17
#define out_3_pin 18

#define tx_reader_module 2
#define rx_reader_module 1

#define RGB_DATA_PIN 47
#define EXTERNAL_LED_RED_PIN 41
#define EXTERNAL_LED_GREEN_PIN 40
#define EXTERNAL_LED_BLUE_PIN 39

#define POWER_PIN 48

const byte LED_ANT_PINS[ant_qtd] = {3, 3, 3, 3};

#define TEST_PIN 0
#else
#error "Invalid ESP_MODEL."
#endif
#endif // PINS_H