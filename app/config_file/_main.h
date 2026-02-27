#include "vars.h"

// ==================== Classe de Configuração ====================
class CONFIG_FILE
{
private:
	// Buffer reutilizado para montar a string de configuração (reduz fragmentação)
	String new_config;

	// Escreve uma linha no arquivo de configuração
	void writeFile(String to_write)
	{
		File file_config = LittleFS.open(config_file, "a");
		if (!file_config)
			return;

		file_config.print("\n" + to_write);
		file_config.close();
	}

	// Limpa o arquivo de configuração
	void clearFile()
	{
		File file_config = LittleFS.open(config_file, "w"); // Trunca o arquivo
		if (!file_config)
			return;
		file_config.close();
	}

	// Salva um parâmetro lido do arquivo
	void save_parameter(String parameter)
	{
		// ==================== Configuração da antena ====================
		if (parameter.startsWith("antena:"))
		{
			const String antena_data_prefix = ",";
			int start_antena_data;

			parameter.replace("antena:", "");

			start_antena_data = parameter.indexOf(antena_data_prefix);
			const int current_ant = parameter.substring(0, start_antena_data).toInt();
			parameter = parameter.substring(start_antena_data + 1);

			start_antena_data = parameter.indexOf(antena_data_prefix);
			String current_active = parameter.substring(0, start_antena_data);
			parameter = parameter.substring(start_antena_data + 1);

			start_antena_data = parameter.indexOf(antena_data_prefix);
			byte current_power = parameter.substring(0, start_antena_data).toInt();
			parameter = parameter.substring(start_antena_data + 1);

			byte current_rssi = parameter.toInt();

			current_power = constrain(current_power, min_power, max_power);
			current_rssi = max(current_rssi, min_rssi);

			antena_commands.set_antena(
				current_ant,
				(current_active == "on"),
				current_power,
				current_rssi);
		}
		// ==================== Sessão ====================
		else if (parameter.startsWith("session:"))
		{
			parameter.replace("session:", "");
			session = parameter.toInt();
			if (session > max_session)
				session = 0x00;
		}
		// ==================== Modos gerais ====================
		else if (parameter.startsWith("start_reading:"))
		{
			parameter.replace("start_reading:", "");
			start_reading = (parameter == "on");
			read_on = start_reading;
		}
		else if (parameter.startsWith("gpi_start:"))
		{
			parameter.replace("gpi_start:", "");
			gpi_start = (parameter == "on");
		}
		else if (parameter.startsWith("gpi_stop_delay:"))
		{
			parameter.replace("gpi_stop_delay:", "");
			gpi_stop_delay = parameter.toInt();
		}
		else if (parameter.startsWith("always_send:"))
		{
			parameter.replace("always_send:", "");
			always_send = (parameter == "on");
		}
		else if (parameter.startsWith("simple_send:"))
		{
			parameter.replace("simple_send:", "");
			simple_send = (parameter == "on");
		}
		else if (parameter.startsWith("hotspot_on:"))
		{
			parameter.replace("hotspot_on:", "");
			hotspot_on = (parameter == "on");
		}
		else if (parameter.startsWith("keyboard:"))
		{
			parameter.replace("keyboard:", "");
			keyboard = (parameter == "on");
		}
		else if (parameter.startsWith("buzzer_on:"))
		{
			buzzer_on = parameter.endsWith("on");
		}
		else if (parameter.startsWith("decode_gtin:"))
		{
			decode_gtin = parameter.endsWith("on");
		}
		else if (parameter.startsWith("dhcp_on:"))
		{
			dhcp_on = parameter.endsWith("on");
		}
		else if (parameter.startsWith("static_ip:"))
		{
			static_ip = parameter.substring(parameter.indexOf(":") + 1);
		}
		else if (parameter.startsWith("gateway_ip:"))
		{
			gateway_ip = parameter.substring(parameter.indexOf(":") + 1);
		}
		else if (parameter.startsWith("subnet_mask:"))
		{
			subnet_mask = parameter.substring(parameter.indexOf(":") + 1);
		}
		// ==================== Webhook ====================
		else if (parameter.startsWith("webhook_on:"))
		{
			webhook_on = parameter.endsWith("on");
		}
		else if (parameter.startsWith("webhook_url:"))
		{
			webhook_url = parameter.substring(parameter.indexOf(":") + 1);
		}
		else if (parameter.startsWith("prefix:"))
		{
			prefix = parameter.substring(parameter.indexOf(":") + 1);
		}
	}

public:
	// Salva toda a configuração no arquivo
	void save_config()
	{
		const int save_time = 10000; // intervalo mínimo entre salvamentos (ms)
		static unsigned long last_save_time = 0;
		static String old_config = "";
		static bool first_time = true;
		static bool reserved_capacity = false; // reserva apenas uma vez

		if ((millis() - last_save_time) < save_time)
			return;

		last_save_time = millis();

		// Na primeira chamada, reservar uma capacidade aproximada para reduzir realocações
		if (!reserved_capacity)
		{
			// Estimativa: ~32 chars por antena + ~512 chars para demais parâmetros
			size_t approx = (size_t)ant_qtd * 32u + 512u;
			new_config.reserve(approx);
			reserved_capacity = true;
		}

		// Limpa o conteúdo mantendo a capacidade reservada
		new_config = "";

		// Monta a configuração atual em uma única string

		// Antenas
		for (int i = 0; i < ant_qtd; i++)
		{
			new_config += "antena:" +
						  String(i + 1) + "," +
						  (antena[i].active ? "on" : "off") + "," +
						  String(antena[i].power) + "," +
						  String(antena[i].rssi) + "\n";
		}

		// Demais parâmetros
		new_config += "session:" + String(session) + "\n";
		new_config += "start_reading:" + String(start_reading ? "on" : "off") + "\n";
		new_config += "gpi_start:" + String(gpi_start ? "on" : "off") + "\n";
		new_config += "gpi_stop_delay:" + String(gpi_stop_delay) + "\n";
		new_config += "always_send:" + String(always_send ? "on" : "off") + "\n";
		new_config += "simple_send:" + String(simple_send ? "on" : "off") + "\n";
		new_config += "hotspot_on:" + String(hotspot_on ? "on" : "off") + "\n";
		new_config += "keyboard:" + String(keyboard ? "on" : "off") + "\n";
		new_config += "buzzer_on:" + String(buzzer_on ? "on" : "off") + "\n";
		new_config += "decode_gtin:" + String(decode_gtin ? "on" : "off") + "\n";
		new_config += "dhcp_on:" + String(dhcp_on ? "on" : "off") + "\n";
		new_config += "static_ip:" + static_ip + "\n";
		new_config += "gateway_ip:" + gateway_ip + "\n";
		new_config += "subnet_mask:" + subnet_mask + "\n";
		// Webhook
		new_config += "webhook_on:" + String(webhook_on ? "on" : "off") + "\n";
		new_config += "webhook_url:" + webhook_url + "\n";
		new_config += "prefix:" + prefix + "\n";
		// Protected Inventory
		new_config += "protected_inventory_enabled:" + String(protected_inventory_enabled ? "on" : "off") + "\n";
		new_config += "protected_inventory_password:" + protected_inventory_password + "\n";

		// Na primeira chamada, apenas inicializa a referência e não salva
		if (first_time)
		{
			old_config = new_config;
			first_time = false;
			return;
		}

		// Se não mudou, não precisa salvar
		if (new_config == old_config)
			return;

		old_config = new_config; // atualiza cache

		// Reescreve o arquivo
		clearFile();

		// Quebra em linhas e salva
		int start = 0, idx;
		while ((idx = new_config.indexOf('\n', start)) != -1)
		{
			String line = new_config.substring(start, idx);
			if (line.length() > 0)
				writeFile(line);
			start = idx + 1;
		}
	}

	// Carrega a configuração do arquivo
	void get_config()
	{
		File file_config = LittleFS.open(config_file, "r");
		if (!file_config)
			return;

		while (file_config.available())
		{
			String line = file_config.readStringUntil('\n');
			line.toLowerCase();
			line.replace(" ", "");
			line.replace("\r", "");
			line.replace("\n", "");
			save_parameter(line);
		}

		file_config.close();
	}
};
