#include "vars.h"
#include <stdlib.h>

// ==================== Classe de Configuração ====================
class CONFIG_FILE
{
private:
	// Buffer reutilizado para montar a string de configuração (reduz fragmentação)
	String new_config;

	// Adler-32 checksum
	uint32_t adler32(const String &s)
	{
		const uint8_t *data = (const uint8_t *)s.c_str();
		size_t len = s.length();
		const uint32_t MOD_ADLER = 65521U;
		uint32_t a = 1, b = 0;
		for (size_t i = 0; i < len; ++i)
		{
			a = (a + data[i]) % MOD_ADLER;
			b = (b + a) % MOD_ADLER;
		}
		return (b << 16) | a;
	}

	// Escreve todo o conteúdo de forma atômica usando arquivo temporário
	bool writeAtomic(const String &content)
	{
		Serial.println("[config] Saving configuration...");
		const char *tmpPath = "/config.tmp";
		const char *mainPath = config_file.c_str();
		const char *bak1 = "/config.bak";
		const char *bak2 = "/config.bak2";

		// Prepara conteúdo com checksum (adler32)
		uint32_t chk = adler32(content);
		char chkbuf[12];
		sprintf(chkbuf, "%08X", (unsigned int)chk);
		String full = content;
		if (!full.endsWith("\n"))
			full += "\n";
		full += String("checksum:") + chkbuf + "\n";

		// Tenta abrir arquivo temporário para escrita
		File f = LittleFS.open(tmpPath, "w");
		if (!f)
		{
			Serial.println("[config] Falha ao abrir arquivo temporario para escrita");
			return false;
		}

		if (f.print(full) < 0)
		{
			Serial.println("[config] Falha ao gravar conteudo no temporario");
			f.close();
			LittleFS.remove(tmpPath);
			return false;
		}
		f.close();

		// Rotaciona backups (bak1 -> bak2, main -> bak1)
		if (LittleFS.exists(bak1))
		{
			if (LittleFS.exists(bak2))
				LittleFS.remove(bak2);
			LittleFS.rename(bak1, bak2);
		}

		if (LittleFS.exists(mainPath))
		{
			if (!LittleFS.rename(mainPath, bak1))
			{
				// Fallback: copia manualmente
				File fm = LittleFS.open(mainPath, "r");
				File fb = LittleFS.open(bak1, "w");
				if (fm && fb)
				{
					while (fm.available())
						fb.write(fm.read());
					fb.close();
					fm.close();
				}
				else
				{
					if (fb)
						fb.close();
					if (fm)
						fm.close();
					Serial.println("[config] Falha ao criar backup (copy fallback)");
				}
			}
		}

		// Substitui o arquivo antigo pelo temporario
		if (!LittleFS.rename(tmpPath, mainPath))
		{
			// Tentativa de fallback: escreve diretamente
			Serial.println("[config] Falha ao renomear tmp -> main, tentando fallback");
			File f2 = LittleFS.open(mainPath, "w");
			if (!f2)
			{
				Serial.println("[config] Fallback falhou: nao foi possivel abrir arquivo principal");
				// Tenta restaurar backup
				if (LittleFS.exists(bak1))
				{
					LittleFS.rename(bak1, mainPath);
				}
				LittleFS.remove(tmpPath);
				return false;
			}
			f2.print(content);
			f2.close();
			LittleFS.remove(tmpPath);
		}

		return true;
	}

	// Remove espaços apenas nas bordas (preserva espaços no valor)
	String trimStr(const String &s)
	{
		String t = s;
		while (t.length() && (t.charAt(0) == ' ' || t.charAt(0) == '\t'))
			t = t.substring(1);
		while (t.length() && (t.charAt(t.length() - 1) == ' ' || t.charAt(t.length() - 1) == '\t'))
			t = t.substring(0, t.length() - 1);
		return t;
	}

	// Salva um parâmetro lido do arquivo
	// Retorna true se o parâmetro foi reconhecido e aplicado
	bool save_parameter(String parameter)
	{
		Serial.println("[config] Processing parameter: " + parameter);
		bool handled = false;

		// ==================== Configuração da antena ====================
		if (parameter.startsWith("antena:"))
		{
			parameter.replace("antena:", "");
			// Esperado: "N,on,POWER,RSSI"
			int p1 = parameter.indexOf(',');
			if (p1 == -1)
				return false; // formato invalido
			int current_ant = parameter.substring(0, p1).toInt();
			if (current_ant < 1 || current_ant > ant_qtd)
				return false;

			String rest = parameter.substring(p1 + 1);
			int p2 = rest.indexOf(',');
			if (p2 == -1)
				return false;
			String current_active = rest.substring(0, p2);

			rest = rest.substring(p2 + 1);
			int p3 = rest.indexOf(',');
			if (p3 == -1)
				return false;
			int current_power = rest.substring(0, p3).toInt();
			rest = rest.substring(p3 + 1);
			int current_rssi = rest.toInt();

			current_power = constrain(current_power, min_power, max_power);
			current_rssi = max((int)current_rssi, (int)min_rssi);

			antena_commands.set_antena(
				current_ant,
				(current_active == "on"),
				current_power,
				current_rssi);

			handled = true;
			return true;
		}
		// ==================== Sessão ====================
		else if (parameter.startsWith("session:"))
		{
			parameter.replace("session:", "");
			session = parameter.toInt();
			if (session > max_session)
				session = 0x00;
			handled = true;
			return true;
		}
		// ==================== Modos gerais ====================
		else if (parameter.startsWith("start_reading:"))
		{
			start_reading = parameter.endsWith("on");
			read_on = start_reading;
			handled = true;
			return true;
		}
		else if (parameter.startsWith("gpi_start:"))
		{
			gpi_start = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("gpi_stop_delay:"))
		{
			parameter.replace("gpi_stop_delay:", "");
			gpi_stop_delay = parameter.toInt();
			handled = true;
			return true;
		}
		else if (parameter.startsWith("always_send:"))
		{
			always_send = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("simple_send:"))
		{
			simple_send = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("hotspot_on:"))
		{
			hotspot_on = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("keyboard:"))
		{
			keyboard = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("buzzer_on:"))
		{
			buzzer_on = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("decode_gtin:"))
		{
			decode_gtin = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("dhcp_on:"))
		{
			dhcp_on = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("static_ip:"))
		{
			static_ip = parameter.substring(parameter.indexOf(":") + 1);
			static_ip = trimStr(static_ip);
			handled = true;
			return true;
		}
		else if (parameter.startsWith("gateway_ip:"))
		{
			gateway_ip = parameter.substring(parameter.indexOf(":") + 1);
			gateway_ip = trimStr(gateway_ip);
			handled = true;
			return true;
		}
		else if (parameter.startsWith("subnet_mask:"))
		{
			subnet_mask = parameter.substring(parameter.indexOf(":") + 1);
			subnet_mask = trimStr(subnet_mask);
			handled = true;
			return true;
		}
		// ==================== Webhook ====================
		else if (parameter.startsWith("webhook_on:"))
		{
			webhook_on = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("webhook_url:"))
		{
			webhook_url = parameter.substring(parameter.indexOf(":") + 1);
			webhook_url = trimStr(webhook_url);
			handled = true;
			return true;
		}
		else if (parameter.startsWith("prefix:"))
		{
			prefix = parameter.substring(parameter.indexOf(":") + 1);
			prefix = trimStr(prefix);
			handled = true;
			return true;
		}

		// Protected Inventory
		else if (parameter.startsWith("protected_inventory_enabled:"))
		{
			protected_inventory_enabled = parameter.endsWith("on");
			handled = true;
			return true;
		}
		else if (parameter.startsWith("protected_inventory_password:"))
		{
			protected_inventory_password = parameter.substring(parameter.indexOf(":") + 1);
			protected_inventory_password = trimStr(protected_inventory_password);
			handled = true;
			return true;
		}

		// ==================== Wi-Fi ====================
		else if (parameter.startsWith("wifi_ssid:"))
		{
			wifi_ssid = parameter.substring(parameter.indexOf(":") + 1);
			wifi_ssid = trimStr(wifi_ssid);
			handled = true;
			return true;
		}
		else if (parameter.startsWith("wifi_password:"))
		{
			wifi_password = parameter.substring(parameter.indexOf(":") + 1);
			wifi_password = trimStr(wifi_password);
			handled = true;
			return true;
		}

		return handled;
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
		// Wi-Fi
		new_config += "wifi_ssid:" + wifi_ssid + "\n";
		new_config += "wifi_password:" + wifi_password + "\n";

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

		// Reescreve o arquivo de forma atômica
		if (!fs_loaded)
		{
			Serial.println("[config] Filesystem nao montado; ignorando tentativa de salvar configuracao");
			return;
		}
		if (!writeAtomic(new_config))
		{
			Serial.println("[config] Falha ao salvar configuracao");
		}
	}

	// Carrega a configuração do arquivo
	void get_config()
	{
		// Se filesystem nao estiver montado, nao tenta ler
		if (!fs_loaded)
		{
			Serial.println("[config] Filesystem nao montado; carregamento de configuracao ignorado");
			return;
		}

		bool triedRestore = false;
		int successes = 0;

		// Tenta abrir o arquivo; se nao existir, tenta restaurar de backup
		if (!LittleFS.exists(config_file.c_str()))
		{
			Serial.println("[config] Arquivo de configuracao nao encontrado");
			if (LittleFS.exists("/config.bak"))
			{
				Serial.println("[config] Tentando restaurar de /config.bak");
				if (LittleFS.rename("/config.bak", config_file.c_str()))
				{
					triedRestore = true;
				}
				else
				{
					Serial.println("[config] Falha ao restaurar backup");
				}
			}
			if (!LittleFS.exists(config_file.c_str()))
			{
				// Se nao existe nenhum arquivo de configuracao, cria um padrao
				Serial.println("[config] Nenhuma configuracao encontrada; criando configuracao padrao");
				String defaults = "session:0\nstart_reading:off\n";
				if (!writeAtomic(defaults))
				{
					Serial.println("[config] Falha ao criar arquivo de configuracao padrao");
				}
				return;
			}
		}

		File file_config = LittleFS.open(config_file, "r");
		if (!file_config)
		{
			Serial.println("[config] Falha ao abrir arquivo de configuracao");
			return;
		}

		// Lê todo o conteúdo para validar checksum (se presente)
		String full = file_config.readString();
		file_config.close();

		full.replace("\r", "");
		// Remove quebras finais
		while (full.length() && full.charAt(full.length() - 1) == '\n')
			full.remove(full.length() - 1);

		int lastNl = full.lastIndexOf('\n');
		String lastLine = (lastNl >= 0) ? full.substring(lastNl + 1) : full;
		bool checksumPresent = false;
		uint32_t expectedChk = 0;
		String dataPart = full;
		if (lastLine.startsWith("checksum:"))
		{
			checksumPresent = true;
			String hex = lastLine.substring(9);
			hex = trimStr(hex);
			unsigned long parsed = strtoul(hex.c_str(), NULL, 16);
			expectedChk = (uint32_t)parsed;
			// dataPart inclui a parte anterior com a quebra final
			if (lastNl >= 0)
				dataPart = full.substring(0, lastNl + 1);
			else
				dataPart = String();

			uint32_t calc = adler32(dataPart);
			if (calc != expectedChk)
			{
				Serial.println("[config] Checksum invalido, tentando restaurar backup");
				// tenta restaurar bak se existir
				if (LittleFS.exists("/config.bak"))
				{
					if (LittleFS.rename("/config.bak", config_file.c_str()))
					{
						// abre novo arquivo e substitui full/dataPart
						File f2 = LittleFS.open(config_file, "r");
						if (f2)
						{
							full = f2.readString();
							f2.close();
							full.replace("\r", "");
							while (full.length() && full.charAt(full.length() - 1) == '\n')
								full.remove(full.length() - 1);
							lastNl = full.lastIndexOf('\n');
							lastLine = (lastNl >= 0) ? full.substring(lastNl + 1) : full;
							if (lastLine.startsWith("checksum:"))
							{
								if (lastNl >= 0)
									dataPart = full.substring(0, lastNl + 1);
								else
									dataPart = String();
							}
						}
					}
					else
					{
						Serial.println("[config] Falha ao renomear backup para restauracao");
					}
				}
				else
				{
					// No backup available
				}
			}
		}

		// Agora processa as linhas da parte de dados (sem a linha checksum)
		int start = 0, idx;
		while ((idx = dataPart.indexOf('\n', start)) != -1)
		{
			String line = dataPart.substring(start, idx);
			if (line.length() == 0)
			{
				start = idx + 1;
				continue;
			}

			int colon = line.indexOf(':');
			if (colon <= 0)
			{
				start = idx + 1;
				continue;
			}

			String key = line.substring(0, colon);
			String value = line.substring(colon + 1);
			key.replace(" ", "");
			key.toLowerCase();
			value = trimStr(value);
			String processed = key + ":" + value;
			if (save_parameter(processed))
				successes++;

			start = idx + 1;
		}

		// Resta possivel linha final sem \n
		if (start < dataPart.length())
		{
			String line = dataPart.substring(start);
			int colon = line.indexOf(':');
			if (colon > 0)
			{
				String key = line.substring(0, colon);
				String value = line.substring(colon + 1);
				key.replace(" ", "");
				key.toLowerCase();
				value = trimStr(value);
				String processed = key + ":" + value;
				if (save_parameter(processed))
					successes++;
			}
		}
	}
};
