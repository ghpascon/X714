#include "vars.h"

// ==================== Classe de Configuração ====================
class CONFIG_FILE
{
private:
	// Buffer reutilizado para montar a string de configuração (reduz fragmentação)
	String new_config;

	// Limpa o arquivo de configuração
	void clearFile()
	{
		File file_config = LittleFS.open(config_file, "w"); // Trunca o arquivo
		if (!file_config)
			return;
		file_config.close();
	}

	// Parse boolean flexível (on/1/true/yes)
	bool parseBool(String v)
	{
		v.trim();
		v.toLowerCase();
		return (v == "on" || v == "1" || v == "true" || v == "yes");
	}

	// Salva um parâmetro lido do arquivo (key/value separados)
	void save_parameter(String key, String value)
	{
		// keys are already lowercase and trimmed
		// ==================== Configuração da antena ====================
		if (key == "antena")
		{
			// form expected: <num>,<on|off>,<power>,<rssi>
			int s1 = value.indexOf(',');
			if (s1 < 0)
				return;
			int s2 = value.indexOf(',', s1 + 1);
			if (s2 < 0)
				return;
			int s3 = value.indexOf(',', s2 + 1);
			if (s3 < 0)
				return;

			String t1 = value.substring(0, s1);		 // antenna index
			String t2 = value.substring(s1 + 1, s2); // active
			String t3 = value.substring(s2 + 1, s3); // power
			String t4 = value.substring(s3 + 1);	 // rssi

			t1.trim();
			t2.trim();
			t3.trim();
			t4.trim();

			int current_ant = t1.toInt();
			bool current_active = parseBool(t2);
			int current_power = t3.toInt();
			int current_rssi = t4.toInt();

			// enforce maximum of 27 as requested
			int effective_max_power = max_power;
			if (effective_max_power > 27)
				effective_max_power = 27;

			current_power = constrain(current_power, (int)min_power, effective_max_power);
			current_rssi = max(current_rssi, (int)min_rssi);

			if (current_ant < 1 || current_ant > ant_qtd)
				return;

			antena_commands.set_antena(
				current_ant,
				current_active,
				(byte)current_power,
				(byte)current_rssi);
		}
		// ==================== Sessão ====================
		else if (key == "session")
		{
			int v = value.toInt();
			if (v > max_session || v < 0)
				v = 0x00;
			session = v;
		}
		// ==================== Modos gerais ====================
		else if (key == "start_reading")
		{
			start_reading = parseBool(value);
			read_on = start_reading;
		}
		else if (key == "gpi_start")
		{
			gpi_start = parseBool(value);
		}
		else if (key == "gpi_stop_delay")
		{
			int v = value.toInt();
			if (v < 0)
				v = 0;
			gpi_stop_delay = v;
		}
		else if (key == "always_send")
		{
			always_send = parseBool(value);
		}
		else if (key == "simple_send")
		{
			simple_send = parseBool(value);
		}
		else if (key == "hotspot_on")
		{
			hotspot_on = parseBool(value);
		}
		else if (key == "keyboard")
		{
			keyboard = parseBool(value);
		}
		else if (key == "buzzer_on")
		{
			buzzer_on = parseBool(value);
		}
		else if (key == "decode_gtin")
		{
			decode_gtin = parseBool(value);
		}
		else if (key == "dhcp_on")
		{
			dhcp_on = parseBool(value);
		}
		else if (key == "static_ip")
		{
			static_ip = value;
		}
		else if (key == "gateway_ip")
		{
			gateway_ip = value;
		}
		else if (key == "subnet_mask")
		{
			subnet_mask = value;
		}
		// ==================== Webhook ====================
		else if (key == "webhook_on")
		{
			webhook_on = parseBool(value);
		}
		else if (key == "webhook_url")
		{
			webhook_url = value;
		}
		else if (key == "prefix")
		{
			// replace spaces with commas as requested
			value.replace(" ", ",");
			prefix = value;
		}
		// Protected Inventory
		else if (key == "protected_inventory_enabled")
		{
			protected_inventory_enabled = parseBool(value);
		}
		else if (key == "protected_inventory_password")
		{
			protected_inventory_password = value;
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

		// Na primeira chamada já tratada acima; agora gravar o novo conteúdo
		// Sanitiza prefix antes de gravar (substitui espaços por vírgulas)
		String to_write = new_config;
		int pidx = to_write.indexOf("prefix:");
		if (pidx >= 0)
		{
			int lineStart = pidx;
			int lineEnd = to_write.indexOf('\n', lineStart);
			if (lineEnd == -1)
				lineEnd = to_write.length();
			String prefixLine = to_write.substring(lineStart, lineEnd);
			String prefixValue = prefixLine.substring(String("prefix:").length());
			prefixValue.trim();
			prefixValue.replace(" ", ",");
			String newPrefixLine = "prefix:" + prefixValue;
			to_write = to_write.substring(0, lineStart) + newPrefixLine + to_write.substring(lineEnd);
		}

		// Grava o arquivo de uma vez, para reduzir operações em flash
		File file_config = LittleFS.open(config_file, "w");
		if (!file_config)
			return;
		file_config.print(to_write);
		file_config.close();
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
			line.replace("\r", "");
			line.trim();
			if (line.length() == 0)
				continue;

			int sep = line.indexOf(':');
			if (sep < 0)
				continue;

			String key = line.substring(0, sep);
			String value = line.substring(sep + 1);
			key.trim();
			key.toLowerCase();
			value.trim();

			// garantir que prefix usa vírgulas em vez de espaços (redundante com save)
			if (key == "prefix")
				value.replace(" ", ",");

			save_parameter(key, value);
		}

		file_config.close();
	}
};
