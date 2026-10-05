#include "vars.h"

class LED_RGB
{
public:
#if INVERT_RGB
#define ON HIGH
#define OFF LOW
#else
#define ON LOW
#define OFF HIGH
#endif

	void setup()
	{
		leds.begin();
		leds.clear();
		leds.show();
		pinMode(EXTERNAL_LED_RED_PIN, OUTPUT);
		pinMode(EXTERNAL_LED_GREEN_PIN, OUTPUT);
		pinMode(EXTERNAL_LED_BLUE_PIN, OUTPUT);
	}

	void state()
	{
		static uint8_t last_state = 0xFF;
		bool connected = is_connected(true);
		uint8_t current_state = (setup_done ? 0x04 : 0x00) | (read_on ? 0x02 : 0x00) | (connected ? 0x01 : 0x00);
		if (current_state == last_state)
			return;
		last_state = current_state;
		byte led_brigthness = 0x50;

		// SETUP
		if (!setup_done)
		{
			leds.setPixelColor(0, leds.Color(led_brigthness, 0x00, 0x00));
			digitalWrite(EXTERNAL_LED_RED_PIN, ON);
			digitalWrite(EXTERNAL_LED_GREEN_PIN, OFF);
			digitalWrite(EXTERNAL_LED_BLUE_PIN, OFF);
		}

		// IDLE
		else if (!read_on)
			if (connected)
			{
				leds.setPixelColor(0, leds.Color(0x00, 0x00, led_brigthness));
				digitalWrite(EXTERNAL_LED_RED_PIN, OFF);
				digitalWrite(EXTERNAL_LED_GREEN_PIN, OFF);
				digitalWrite(EXTERNAL_LED_BLUE_PIN, ON);
			}
			else
			{
				leds.setPixelColor(0, leds.Color(led_brigthness, led_brigthness, 0x00));
				digitalWrite(EXTERNAL_LED_RED_PIN, ON);
				digitalWrite(EXTERNAL_LED_GREEN_PIN, ON);
				digitalWrite(EXTERNAL_LED_BLUE_PIN, OFF);
			}

		// READING
		else if (connected)
		{
			leds.setPixelColor(0, leds.Color(0x00, led_brigthness, led_brigthness));
			digitalWrite(EXTERNAL_LED_RED_PIN, OFF);
			digitalWrite(EXTERNAL_LED_GREEN_PIN, ON);
			digitalWrite(EXTERNAL_LED_BLUE_PIN, ON);
		}
		else
		{
			leds.setPixelColor(0, leds.Color(0x00, led_brigthness, 0x00));
			digitalWrite(EXTERNAL_LED_RED_PIN, OFF);
			digitalWrite(EXTERNAL_LED_GREEN_PIN, ON);
			digitalWrite(EXTERNAL_LED_BLUE_PIN, OFF);
		}

		leds.show();
	}
};
