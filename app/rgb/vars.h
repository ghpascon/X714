#include <Adafruit_NeoPixel.h>
#define NUM_LEDS 1
Adafruit_NeoPixel leds(NUM_LEDS, RGB_DATA_PIN, NEO_GRB + NEO_KHZ800);


extern bool setup_done;
extern bool read_on;
extern bool eth_connected;
extern bool wifi_connected;
extern bool btConnected;