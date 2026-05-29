class output_func
{
public:
	unsigned long buzzer_time = millis();
	void set_outputs()
	{
		set_gpos();
		set_buzzer();
		set_ant_leds();
	}

	void set_gpos()
	{
		for (int i = 0; i < gpo_qtd; i++)
		{
			bool want_output = gpo[i];
			if (want_output != prev_gpo_output[i])
			{
				prev_gpo_output[i] = want_output;
				if (want_output)
					pinMode(gpo_pin[i], OUTPUT);
				else
					pinMode(gpo_pin[i], INPUT_PULLUP);
			}
		}
	}

	void set_buzzer()
	{
		const unsigned long now = millis();
		if (now < 2000)
			return;
		const int buzzer_time_on = 200;
		const int indicator_time_on = 1000;
		// prev_buzzer_state and prev_indicator_output are now members

		bool buzzer_should_be_on = (now - buzzer_time < (unsigned long)buzzer_time_on) && buzzer_on;
		if (buzzer_should_be_on != prev_buzzer_state)
		{
			prev_buzzer_state = buzzer_should_be_on;
			digitalWrite(buzzer_pin, buzzer_should_be_on ? HIGH : LOW);
		}

		bool indicator_should_be_output = (now - buzzer_time < (unsigned long)indicator_time_on) && buzzer_on;
		if (indicator_should_be_output != prev_indicator_output)
		{
			prev_indicator_output = indicator_should_be_output;
			if (indicator_should_be_output)
				pinMode(indicator_pin, OUTPUT);
			else
				pinMode(indicator_pin, INPUT_PULLUP);
		}
	}

	void set_ant_leds()
	{
		const unsigned long now = millis();
		for (int i = 0; i < ant_qtd; i++)
		{
			bool desired = setup_done ? (now - last_ant_led_update[i] < (unsigned long)ant_led_time) : false;
			if (desired != prev_led_state[i])
			{
				prev_led_state[i] = desired;
				digitalWrite(LED_ANT_PINS[i], desired ? HIGH : LOW);
			}
		}
	}

	void trigger_ant_led(int ant_index)
	{
		if (ant_index < 1 || ant_index > ant_qtd)
			return;
		last_ant_led_update[ant_index - 1] = millis();
	}

	void test_all_outputs(int time = 500)
	{
		// Activate all GPOs (open-drain: drive LOW to activate)
		for (int i = 0; i < gpo_qtd; i++)
		{
			pinMode(gpo_pin[i], OUTPUT);
		}

		// Activate buzzer and indicator
		digitalWrite(buzzer_pin, HIGH);
		pinMode(indicator_pin, OUTPUT);

		// Activate antenna LEDs
		for (int i = 0; i < ant_qtd; i++)
		{
			digitalWrite(LED_ANT_PINS[i], HIGH);
		}

		delay(time);

		// Deactivate all GPOs
		for (int i = 0; i < gpo_qtd; i++)
		{
			gpo[i] = false;
			prev_gpo_output[i] = false;
			pinMode(gpo_pin[i], INPUT_PULLUP);
		}

		// Deactivate buzzer and indicator
		buzzer_on = false;
		prev_buzzer_state = false;
		digitalWrite(buzzer_pin, LOW);
		prev_indicator_output = false;
		pinMode(indicator_pin, INPUT_PULLUP);

		// Deactivate antenna LEDs and reset timers
		for (int i = 0; i < ant_qtd; i++)
		{
			prev_led_state[i] = false;
			digitalWrite(LED_ANT_PINS[i], LOW);
			last_ant_led_update[i] = 0;
		}
	}

private:
	bool prev_gpo_output[gpo_qtd] = {};
	bool prev_led_state[ant_qtd] = {};
	bool prev_buzzer_state = false;
	bool prev_indicator_output = false;
	unsigned long last_ant_led_update[ant_qtd] = {};
	const int ant_led_time = 200;
};