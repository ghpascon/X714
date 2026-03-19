#include "vars.h"
#include "input_func.h"
#include "output_func.h"

class PINS : public input_func, public output_func
{
public:
	void setup()
	{
		pinMode(in_1_pin, INPUT_PULLUP);
		pinMode(in_2_pin, INPUT_PULLUP);
		pinMode(in_3_pin, INPUT_PULLUP);

		pinMode(gpo_pin[0], INPUT_PULLUP);
		pinMode(gpo_pin[1], INPUT_PULLUP);
		pinMode(gpo_pin[2], INPUT_PULLUP);

		pinMode(buzzer_pin, OUTPUT);
		pinMode(indicator_pin, INPUT_PULLUP);
	}

	void write_gpo(int index, bool state)
	{
		index -= 1; // Adjust index to be 0-based
		if (index < 0 || index >= gpo_qtd)
			return;

		gpo[index] = state;
	}
};
