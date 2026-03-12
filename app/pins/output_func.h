class output_func
{
public:
	long buzzer_time = millis();
	void set_outputs()
	{
		set_gpos();
		set_buzzer();
	}

	void set_gpos()
	{
		for (int i = 0; i < gpo_qtd; i++)
			gpo[i]
				? pinMode(gpo_pin[i], INPUT_PULLUP)
				: pinMode(gpo_pin[i], OUTPUT);
	}

	void set_buzzer()
	{
		if (millis() < 2000)
			return;
		const int buzzer_time_on = 200;
		const int indicator_time_on = 1000;
		(millis() - buzzer_time < buzzer_time_on && buzzer_on) ? digitalWrite(buzzer_pin, HIGH) : digitalWrite(buzzer_pin, LOW);
		(millis() - buzzer_time < indicator_time_on && buzzer_on) ? pinMode(indicator_pin, OUTPUT) : pinMode(indicator_pin, INPUT_PULLUP);
	}
};