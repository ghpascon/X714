#include "vars.h"
#include "reader_commands.h"
#include "reader_periodic.h"
#include "reader_read_commands.h"
#include "reader_serial.h"
#include "reader_setup.h"
#include "reader_verifications.h"
#include "reader_write_commands.h"
#include "protected_mode.h"
class ANTENA
{
public:
	bool active;
	byte power;
};

class READER : public serial_reader, public reader_read_on_commands, public reader_write_commands, public setup_commands_reader, public periodic_commands_reader, public reader_verifications, public protected_mode
{
public:
	ANTENA antena[settings.ant_qtd];
	int rx_pin;
	int tx_pin;

	READER(int rx, int tx) : rx_pin(rx), tx_pin(tx)
	{
	}
	void setup()
	{
		Serial2.begin(115200, SERIAL_8N1, rx_pin, tx_pin);
		set_antennas_by_cfg()
	}

	void functions()
	{
		check_serial();
		reader_verify();

		if (!answer_rec)
			return;

		if (!setup_done)
		{
			config_reader();
			return;
		}

		if (read_on && answer_rec)
		{
			read_on_command();
			reader_band();
		}
	}

	void setup_reader()
	{
		setup_done = false;
		step = 0;
	}

	void config_reader()
	{
		if (step == 0)
			start_reader();
		else if (step == 1)
			set_reader_adress(0x00);
		else if (step == 2)
			reader_band();
		else if (step == 3)
			reader_work_mode();
		else if (step == 4)
			reader_session();
		else if (step == 5)
			set_active_ant();
		else if (step == 6)
			set_ant_power();
		else if (step == 7)
			set_ant_check();
		else if (step == 8)
			set_reader_time();
		else if (step == 9)
			set_ant_pulse();
		else if (step == 10)
			set_retry_write(0x03);
		else if (step == 11)
			query_parameters();
		else if (step == 12)
			set_write_power(antena[0].power);
		else if (step == 13)
			set_rf_link();
		else if (step == 14)
			set_rf_link_gen2x();
		else if (step == 15)
			set_tag_focus();
		else if (step == 16)
			protected_inventory(protected_inventory_enabled, protected_inventory_password);
		else
		{
			myserial.write("#SETUP_DONE");
			setup_done = true;
		}
	}

	// ANTENA
	String get_ant_cfg()
	{
		String cfg = "";
		for (int i = 0; i < settings.ant_qtd; i++)
		{
			cfg += String(i + 1) + ":" + String(antena[i].active ? "1" : "0") + "-" + String(antena[i].power);
			if (i < settings.ant_qtd - 1)
				cfg += ";";
		}
		return cfg;
	}

	void set_antena(int num, bool active, byte power)
	{
		if (num < 1 || num > settings.ant_qtd)
			return;

		power = constrain(power, settings.min_power, settings.max_power);

		antena[num - 1].active = active;
		antena[num - 1].power = power;
		settings.set_ant_cfg(get_ant_cfg());
	}

	void set_antennas_by_cfg()
	{
		String cfg = settings.ant_cfg;
		int start = 0;
		while (start < cfg.length())
		{
			int end = cfg.indexOf(';', start);
			String ant_cfg = (end == -1) ? cfg.substring(start) : cfg.substring(start, end);
			ant_cfg.trim();
			if (ant_cfg.length() > 0)
			{
				int colon = ant_cfg.indexOf(':');
				int dash = ant_cfg.indexOf('-');
				if (colon > 0 && dash > colon)
				{
					int num = ant_cfg.substring(0, colon).toInt();
					bool active = ant_cfg.substring(colon + 1, dash).toInt() > 0;
					byte power = (byte)ant_cfg.substring(dash + 1).toInt();
					set_antena(num, active, power);
				}
			}
			if (end == -1)
				break;
			start = end + 1;
		}
	}

	void set_power_all(byte set_power)
	{
		set_power = constrain(set_power, settings.min_power, settings.max_power);

		for (int i = 0; i < settings.ant_qtd; i++)
		{
			antena[i].power = set_power;
		}

		myserial.write("#READ_POWER:" + String(set_power, DEC));
		settings.set_ant_cfg(get_ant_cfg());
	}

	void decrease_power(byte qtd = 1)
	{
		for (int i = 0; i < settings.ant_qtd; i++)
		{
			antena[i].power = max((int)antena[i].power - (int)qtd, (int)settings.min_power);
		}
		settings.set_ant_cfg(get_ant_cfg());
	}
};