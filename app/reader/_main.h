#include "vars.h"
#include "reader_commands.h"
#include "reader_periodic.h"
#include "reader_read_commands.h"
#include "reader_serial.h"
#include "reader_setup.h"
#include "reader_verifications.h"
#include "reader_write_commands.h"
#include "protected_mode.h"

class READER : public serial_reader, public reader_read_on_commands, public reader_write_commands, public setup_commands_reader, public periodic_commands_reader, public reader_verifications, public protected_mode
{
public:
	void setup()
	{
		Serial2.begin(115200, SERIAL_8N1, rx_reader_module, tx_reader_module);
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
};