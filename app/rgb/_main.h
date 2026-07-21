#include "vars.h"

class LED_RGB
{
public:
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
		static String last_state = "";
		bool connected = is_connected(true);
		String state = String(setup_done) + String(read_on) + String(connected);
		if (state == last_state)
			return;
		last_state = state;
		byte led_brigthness = 0x50;

		// SETUP
		if (!setup_done)
		{
			leds.setPixelColor(0, leds.Color(led_brigthness, 0x00, 0x00));
			digitalWrite(EXTERNAL_LED_RED_PIN, LOW);
			digitalWrite(EXTERNAL_LED_GREEN_PIN, HIGH);
			digitalWrite(EXTERNAL_LED_BLUE_PIN, HIGH);
		}

		// IDLE
		else if (!read_on)
			if (connected)
			{
				leds.setPixelColor(0, leds.Color(0x00, 0x00, led_brigthness));
				digitalWrite(EXTERNAL_LED_RED_PIN, HIGH);
				digitalWrite(EXTERNAL_LED_GREEN_PIN, HIGH);
				digitalWrite(EXTERNAL_LED_BLUE_PIN, LOW);
			}
			else
			{
				leds.setPixelColor(0, leds.Color(led_brigthness, led_brigthness, 0x00));
				digitalWrite(EXTERNAL_LED_RED_PIN, LOW);
				digitalWrite(EXTERNAL_LED_GREEN_PIN, LOW);
				digitalWrite(EXTERNAL_LED_BLUE_PIN, HIGH);
			}

		// READING
		else if (connected)
		{
			leds.setPixelColor(0, leds.Color(0x00, led_brigthness, led_brigthness));
			digitalWrite(EXTERNAL_LED_RED_PIN, HIGH);
			digitalWrite(EXTERNAL_LED_GREEN_PIN, LOW);
			digitalWrite(EXTERNAL_LED_BLUE_PIN, LOW);
		}
		else
		{
			leds.setPixelColor(0, leds.Color(0x00, led_brigthness, 0x00));
			digitalWrite(EXTERNAL_LED_RED_PIN, HIGH);
			digitalWrite(EXTERNAL_LED_GREEN_PIN, LOW);
			digitalWrite(EXTERNAL_LED_BLUE_PIN, HIGH);
		}

		leds.show();
	}
};
