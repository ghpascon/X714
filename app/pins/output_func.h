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
		static bool prev_gpo_output[gpo_qtd] = {};
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
		static bool prev_buzzer_state = false;
		static bool prev_indicator_output = false;

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
		static bool prev_led_state[ant_qtd] = {};
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

private:
	unsigned long last_ant_led_update[ant_qtd] = {};
	const int ant_led_time = 200;
};