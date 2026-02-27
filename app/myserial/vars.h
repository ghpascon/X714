#include "USB.h"
#include "USBHIDKeyboard.h"
USBHIDKeyboard my_keyboard;
USBCDC my_usb;

extern bool keyboard;
extern bool simple_send;

extern CONNECTION connection;
extern bool eth_connected;
extern bool btConnected;