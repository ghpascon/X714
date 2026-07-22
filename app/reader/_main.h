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
		delay(300);
		// flush
		while (Serial2.available())
			Serial2.read();
	}

	void functions()
	{
		check_update_antena();
		check_serial();
		reader_verify();

		if (request_clear_serial_buffers)
		{
			clear_serial_buffers();
			request_clear_serial_buffers = false;
		}

		if (!setup_done && setup_transition_until_ms > 0)
		{
			if (millis() < setup_transition_until_ms)
				return;

			// End of transition window: discard trailing inventory frames so
			// the first setup command starts on a clean serial state.
			clear_serial_buffers();
			setup_transition_until_ms = 0;
			answer_rec = true;
		}

		if (!answer_rec)
			return;

		if (!setup_done)
		{
			config_reader();
			return;
		}

		// periodic_reader_loop();
		clear_tags_no_read();

		if (read_on)
		{
			read_on_functions();
			// Only send band-toggle if the read command didn't already
			// consume the answer slot this cycle.
			reader_band();
		}
	}

	void setup_reader(byte _step = 0)
	{
		setup_done = false;
		step = _step;
		expected_setup_ack_cmd = 0x00;
		last_wait_cmd_sent_ms = 0;
		// Discard any bytes from the previous session so they
		// don't corrupt the first frame of the new setup sequence.
		clear_serial_buffers();
		// Brief guard window to let in-flight inventory frames settle before
		// issuing setup command step 0.
		setup_transition_until_ms = millis() + 120;
		// Ensure setup can start immediately, even if we were waiting
		// for a response that is now irrelevant (e.g. a power change
		// arrived mid-command).
		answer_rec = true;
	}

	void config_reader()
	{
		static byte last_step = 0;
		byte expected_cmd = 0x00;
		if (last_step != step)
		{
			last_step = step;
			myserial.write("#STEP:" + String(step));
		}
		if (step == 0)
		{
			expected_cmd = 0x21;
			start_reader();
		}
		else if (step == 1)
		{
			expected_cmd = 0x24;
			set_reader_adress(0x00);
		}
		else if (step == 2)
		{
			expected_cmd = 0x22;
			reader_band();
		}
		else if (step == 3)
		{
			expected_cmd = 0x76;
			reader_work_mode();
		}
		else if (step == 4)
		{
			expected_cmd = 0x75;
			reader_session();
		}
		else if (step == 5)
		{
			expected_cmd = 0xea;
			set_tag_focus();
		}
		else if (step == 6)
		{
			expected_cmd = 0xea;
			protected_inventory(protected_inventory_enabled, protected_inventory_password);
		}
		else if (step == 7)
		{
			expected_cmd = 0x66;
			set_ant_check();
		}
		else if (step == 8)
		{
			expected_cmd = 0x25;
			set_reader_time();
		}
		else if (step == 9)
		{
			expected_cmd = 0x48;
			set_ant_pulse();
		}
		else if (step == 10)
		{
			expected_cmd = 0x7b;
			set_retry_write(0x03);
		}
		else if (step == 11)
		{
			expected_cmd = 0xea;
			query_parameters();
		}
		else if (step == 12)
		{
			expected_cmd = 0x79;
			set_write_power(max(27, (int)antena[0].power)); // Write power must be at least 27dBm, otherwise tag write may fail. This is a reader limitation, not a tag one.
		}
		else if (step == 13)
		{
			expected_cmd = 0x7f;
			set_rf_link();
		}
		else if (step == 14)
		{
			expected_cmd = 0x7f;
			set_rf_link_gen2x();
		}
		else if (step == 15)
		{
			expected_cmd = 0x3f;
			set_active_ant();
		}
		else if (step == 16)
		{
			expected_cmd = 0x2f;
			if (one_ant)
				set_ant_power();
			else
				set_ant_power_all();
		}
		else
		{
			myserial.write("#SETUP_DONE");
			myserial.write("#NAME:" + get_esp_name());
			myserial.write("#VERSION:" + String(VERSION));
			setup_done = true;
			had_valid_frame = true; // confirmed reader speaks 115200: skip baud-change on reconnect
			reconnect_count = 0;
			expected_setup_ack_cmd = 0x00;
			last_wait_cmd_sent_ms = 0;
			setup_transition_until_ms = 0;
			return; // no command sent — do NOT clear answer_rec
		}

		// Guarantee we wait for the ack of the command just sent,
		// even for functions that use wait_answer=false (e.g. set_tag_focus,
		// protected_inventory). Without this they would be re-sent on every
		// loop until the response eventually arrives.
		expected_setup_ack_cmd = expected_cmd;
		last_wait_cmd_sent_ms = millis();
		answer_rec = false;
	}

	void check_update_antena()
	{
		if (!antena_commands.need_update_antena)
			return;

		antena_commands.need_update_antena = false;

		if (setup_done)
		{
			// Restart setup so the new antenna settings are applied.
			setup_reader();
		}
		// If setup is already running, antena[].power was updated in-place;
		// step 16 (set_ant_power) will read the latest value when it runs.
		// Restarting now would only cause cascading restarts on rapid changes.
	}

	void periodic_reader_loop()
	{
		const int periodic_time = 120000;
		static unsigned long current_periodic_time = -periodic_time;
		if (millis() - current_periodic_time < periodic_time)
			return;
		current_periodic_time = millis();
		get_temp();
	}

	void clear_tags_no_read()
	{
		const unsigned long clear_time = 300000;
		static unsigned long current_clear_time = millis();
		static int last_tag_count = -1;
		const int current_count = tag_commands.tagCount();
		if (current_count != last_tag_count)
		{
			last_tag_count = current_count;
			current_clear_time = millis();
		}

		if (millis() - current_clear_time >= clear_time)
		{
			tag_commands.clear_tags();
			current_clear_time = millis();
		}
	}

	void read_on_functions()
	{
		static unsigned long last_read_on_time = 0;
		const unsigned long read_on_interval = 50;
		unsigned long now = millis();
		if (now - last_read_on_time >= read_on_interval)
		{
			last_read_on_time = now;
			read_on_command();
		}
	}
};