#include "vars.h"
#include <ArduinoJson.h>

static const char *WEB_AUTH_USER = "smartx";
static const char *WEB_AUTH_PASSWORD = "smartx12345";
static const unsigned long AUTH_TIMEOUT_MS = 5UL * 60UL * 1000UL; // 5 minutos
static unsigned long auth_granted_at = 0;

bool ensure_html_route_auth()
{
    if (auth_granted_at > 0 && (millis() - auth_granted_at) < AUTH_TIMEOUT_MS)
        return true;

    if (!server.authenticate(WEB_AUTH_USER, WEB_AUTH_PASSWORD))
    {
        server.requestAuthentication();
        return false;
    }

    auth_granted_at = millis();
    return true;
}

static void send_json_response(int http_code, const JsonDocument &doc)
{
    String body;
    serializeJson(doc, body);
    server.send(http_code, "application/json", body);
}

static void send_status_ok(int http_code = 200)
{
    StaticJsonDocument<64> doc;
    doc["status"] = "ok";
    send_json_response(http_code, doc);
}

static void send_status_error(int http_code, const char *message)
{
    StaticJsonDocument<160> doc;
    doc["status"] = "error";
    doc["message"] = message;
    send_json_response(http_code, doc);
}

static bool parse_json_body(JsonDocument &doc)
{
    if (!server.hasArg("plain"))
    {
        send_status_error(400, "missing json body");
        return false;
    }

    const String body = server.arg("plain");
    if (body.length() == 0)
    {
        send_status_error(400, "empty json body");
        return false;
    }

    const DeserializationError err = deserializeJson(doc, body);
    if (err)
    {
        send_status_error(400, "invalid json body");
        return false;
    }

    return true;
}

static bool serve_fs_file(const char *file_path, const char *content_type)
{
    if (!ensure_html_route_auth())
        return false;

    if (!fs_loaded)
    {
        server.send(503, "text/plain", "Filesystem not mounted");
        return false;
    }

    File f = LittleFS.open(file_path, "r");
    if (!f)
    {
        server.send(404, "text/plain", "Not found");
        return false;
    }

    server.streamFile(f, content_type);
    f.close();
    return true;
}

class WEB_SERVER
{
public:
    void setup()
    {
        if (!hotspot_on && wifi_ssid.length() == 0)
            return;
        config_web_server();
        api_routes();

        server.begin();
    }

    void config_web_server()
    {
        if (wifi_ssid.length() > 0)
        {
            // Modo Station: conecta à rede Wi-Fi configurada
            Serial.println("[wifi] Iniciando modo Station com SSID: " + wifi_ssid);
            WiFi.disconnect(true);
            WiFi.mode(WIFI_STA);
            WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
        }
        else
        {
            // Modo AP: inicia o hotspot
            const String ssid = hotspot_name.length() > 0 ? hotspot_name : get_esp_name();
            const char *password = "smartx12345";
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP);
            WiFi.setTxPower(WIFI_POWER_11dBm);
            WiFi.softAP(ssid.c_str(), password, 6, false, 2);
        }
    }

    void api_routes()
    {
        // Static assets
        server.on("/style.css", HTTP_GET, []()
                  { serve_fs_file("/html/style.css", "text/css"); });

        server.on("/FONT_SMARTX.woff", HTTP_GET, []()
                  { serve_fs_file("/html/FONT_SMARTX.woff", "font/woff"); });

        // HTML pages
        server.on("/", HTTP_GET, []()
                  { serve_fs_file("/html/home.html", "text/html"); });

        server.on("/reader", HTTP_GET, []()
                  { serve_fs_file("/html/reader.html", "text/html"); });

        server.on("/ant_config", HTTP_GET, []()
                  { serve_fs_file("/html/ant_config.html", "text/html"); });

        server.on("/reader_config", HTTP_GET, []()
                  { serve_fs_file("/html/reader_config.html", "text/html"); });

        server.on("/reader_modes", HTTP_GET, []()
                  { serve_fs_file("/html/reader_modes.html", "text/html"); });

        server.on("/gpo_test", HTTP_GET, []()
                  { serve_fs_file("/html/gpo_test.html", "text/html"); });

        server.on("/eth_config", HTTP_GET, []()
                  { serve_fs_file("/html/eth_config.html", "text/html"); });

        server.on("/webhook_config", HTTP_GET, []()
                  { serve_fs_file("/html/webhook_config.html", "text/html"); });

        server.on("/prefix", HTTP_GET, []()
                  { serve_fs_file("/html/prefix.html", "text/html"); });

        server.on("/protected_inventory", HTTP_GET, []()
                  { serve_fs_file("/html/protected_inventory.html", "text/html"); });

        server.on("/wifi_config", HTTP_GET, []()
                  { serve_fs_file("/html/wifi_config.html", "text/html"); });

        // API health
        server.on("/api/v1/status", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;
                      send_status_ok(); });

        // Device info
        server.on("/api/v1/device/info", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<512> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["name"] = get_esp_name();
                      data["bt_mac"] = get_bt_mac();
                      data["eth_mac"] = String(ETH.macAddress());
                      data["ip"] = get_connected_ip();
                      data["version"] = String(VERSION);
                      data["power"] = pins.get_power_supply_state();
                      send_json_response(200, doc); });

        server.on("/api/v1/device/restart", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;
                      send_status_ok();
                      delay(150);
                      ESP.restart(); });

        // Reader data
        server.on("/api/v1/reader/state", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<384> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["reading"] = read_on;
                      data["reading_label"] = read_on ? "ON" : "OFF";
                      data["temperature"] = temperatura;
                      data["total_tags"] = tag_commands.tagCount();
                      data["button_label"] = gpi_start ? "GPI" : (read_on ? "STOP" : "READ");
                      data["button_enabled"] = !gpi_start;
                      send_json_response(200, doc); });

        server.on("/api/v1/reader/toggle-reading", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;
                      if (!gpi_start)
                          read_on = !read_on;
                      send_status_ok(); });

        server.on("/api/v1/reader/clear-tags", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;
                      tag_commands.clear_tags();
                      send_status_ok(); });

        server.on("/api/v1/reader/tags", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      const int snapshot_limit = 30;
                      const int total = tag_commands.tagCount();
                      const int count = total < snapshot_limit ? total : snapshot_limit;

                      DynamicJsonDocument doc(12288);
                      doc["status"] = "ok";
                      JsonArray data = doc.createNestedArray("data");

                      for (int i = 0; i < count; i++)
                      {
                          const TagRecord *record = tag_commands.getTag(i);
                          if (!record)
                              break;

                          JsonObject row = data.createNestedObject();
                          row["index"] = i + 1;
                          row["epc"] = String(record->epc);
                          row["tid"] = String(record->tid);
                          row["antenna"] = record->ant_number;
                          row["rssi"] = record->rssi;
                      }

                      send_json_response(200, doc); });

        // Reader modes
        server.on("/api/v1/reader/modes", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<512> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["start_reading"] = start_reading;
                      data["gpi_start"] = gpi_start;
                      data["always_send"] = always_send;
                      data["simple_send"] = simple_send;
                      data["keyboard"] = keyboard;
                      data["buzzer_on"] = buzzer_on;
                      data["decode_gtin"] = decode_gtin;
                      data["indicator_time"] = indicator_time;
                      send_json_response(200, doc); });

        server.on("/api/v1/reader/modes", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<512> body;
                      if (!parse_json_body(body))
                          return;

                      if (!body["start_reading"].isNull())
                      {
                          start_reading = body["start_reading"].as<bool>();
                          if (start_reading)
                              gpi_start = false;
                      }

                      if (!body["gpi_start"].isNull())
                      {
                          const bool requested_gpi_start = body["gpi_start"].as<bool>();
                          if (start_reading && requested_gpi_start)
                          {
                              send_status_error(409, "start_reading active");
                              return;
                          }
                          gpi_start = requested_gpi_start;
                      }

                      if (!body["always_send"].isNull())
                          always_send = body["always_send"].as<bool>();

                      if (!body["simple_send"].isNull())
                          simple_send = body["simple_send"].as<bool>();

                      if (!body["keyboard"].isNull())
                          keyboard = body["keyboard"].as<bool>();

                      if (!body["buzzer_on"].isNull())
                          buzzer_on = body["buzzer_on"].as<bool>();

                      if (!body["decode_gtin"].isNull())
                          decode_gtin = body["decode_gtin"].as<bool>();

                      if (!body["indicator_time"].isNull())
                      {
                          const int next_indicator_time = body["indicator_time"].as<int>();
                          if (next_indicator_time < 0)
                          {
                              send_status_error(400, "invalid indicator_time");
                              return;
                          }
                          indicator_time = next_indicator_time;
                      }

                      send_status_ok(); });

        // Reader config
        server.on("/api/v1/reader/config", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<256> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["simple_send"] = simple_send;
                      data["session"] = session;
                      data["gpi_stop_delay"] = gpi_stop_delay;
                      send_json_response(200, doc); });

        server.on("/api/v1/reader/config", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<256> body;
                      if (!parse_json_body(body))
                          return;

                      if (!body["simple_send"].isNull())
                          simple_send = body["simple_send"].as<bool>();

                      if (!body["session"].isNull())
                      {
                          int parsed_session = body["session"].as<int>();
                          if (parsed_session < 0 || parsed_session > max_session)
                              parsed_session = 0;
                          session = parsed_session;
                      }

                      if (!body["gpi_stop_delay"].isNull())
                          gpi_stop_delay = body["gpi_stop_delay"].as<int>();

                      reader_module.setup_reader();
                      send_status_ok(); });

        // Antenna config
        server.on("/api/v1/antenna/config", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      DynamicJsonDocument doc(1024);
                      doc["status"] = "ok";
                      JsonArray data = doc.createNestedArray("data");

                      for (int i = 0; i < ant_qtd; i++)
                      {
                          JsonObject ant = data.createNestedObject();
                          ant["id"] = i + 1;
                          ant["active"] = antena[i].active == 1;
                          ant["power"] = antena[i].power;
                          ant["rssi"] = antena[i].rssi;
                      }

                      send_json_response(200, doc); });

        server.on("/api/v1/antenna/config", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      DynamicJsonDocument body(2048);
                      if (!parse_json_body(body))
                          return;

                      JsonArray antennas = body["antennas"].as<JsonArray>();
                      if (antennas.isNull())
                      {
                          send_status_error(400, "missing antennas array");
                          return;
                      }

                      for (int i = 0; i < ant_qtd; i++)
                      {
                          antena[i].active = 0;
                      }

                      for (JsonObject item : antennas)
                      {
                          const int id = item["id"].as<int>();
                          if (id < 1 || id > ant_qtd)
                              continue;

                          const int index = id - 1;
                          if (!item["active"].isNull())
                              antena[index].active = item["active"].as<bool>() ? 1 : 0;
                          if (!item["power"].isNull())
                              antena[index].power = item["power"].as<int>();
                          if (!item["rssi"].isNull())
                              antena[index].rssi = item["rssi"].as<int>();
                      }

                      for (int i = 0; i < ant_qtd; i++)
                      {
                          antena[i].rssi = max(antena[i].rssi, min_rssi);
                          antena[i].power = constrain(antena[i].power, min_power, max_power);
                      }

                      reader_module.setup_reader();
                      send_status_ok(); });

        // GPO
        server.on("/api/v1/gpo", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<256> doc;
                      doc["status"] = "ok";
                      JsonArray data = doc.createNestedArray("data");
                      for (int i = 0; i < gpo_qtd; i++)
                      {
                          data.add(gpo[i]);
                      }
                      send_json_response(200, doc); });

        server.on("/api/v1/gpo/toggle", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<128> body;
                      if (!parse_json_body(body))
                          return;

                      const int index = body["index"].as<int>();
                      if (index < 1 || index > gpo_qtd)
                      {
                          send_status_error(400, "invalid gpo index");
                          return;
                      }

                      const int gpo_index = index - 1;
                      gpo[gpo_index] = !gpo[gpo_index];
                      send_status_ok(); });

        server.on("/api/v1/toggle_buzzer", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;
			          pins.turn_on_buzzer();
                      send_status_ok(); });
        // Ethernet
        server.on("/api/v1/network/ethernet", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<384> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["dhcp_on"] = dhcp_on;
                      data["static_ip"] = static_ip;
                      data["gateway_ip"] = gateway_ip;
                      data["subnet_mask"] = subnet_mask;
                      data["connected"] = eth_connected;
                      data["current_ip"] = eth_connected ? ETH.localIP().toString() : "-";
                      send_json_response(200, doc); });

        server.on("/api/v1/network/ethernet", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<512> body;
                      if (!parse_json_body(body))
                          return;

                      if (!body["dhcp_on"].isNull())
                          dhcp_on = body["dhcp_on"].as<bool>();

                      if (!dhcp_on)
                      {
                          if (!body["static_ip"].isNull())
                              static_ip = body["static_ip"].as<String>();
                          if (!body["gateway_ip"].isNull())
                              gateway_ip = body["gateway_ip"].as<String>();
                          if (!body["subnet_mask"].isNull())
                              subnet_mask = body["subnet_mask"].as<String>();
                      }

                      connection.setup();
                      send_status_ok(); });

        // Wi-Fi
        server.on("/api/v1/network/wifi", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<384> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["wifi_ssid"] = wifi_ssid;
                      data["wifi_password"] = wifi_password;
                      data["connected"] = wifi_connected;
                      data["current_ip"] = wifi_connected ? WiFi.localIP().toString() : "-";
                      send_json_response(200, doc); });

        server.on("/api/v1/network/wifi", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<384> body;
                      if (!parse_json_body(body))
                          return;

                      if (!body["wifi_ssid"].isNull())
                          wifi_ssid = body["wifi_ssid"].as<String>();
                      if (!body["wifi_password"].isNull())
                          wifi_password = body["wifi_password"].as<String>();

                      if (wifi_ssid.length() > 0)
                      {
                          WiFi.disconnect(true);
                          WiFi.mode(WIFI_STA);
                          WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
                      }

                      send_status_ok(); });

        // Webhook
        server.on("/api/v1/webhook", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<256> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["webhook_on"] = webhook_on;
                      data["webhook_url"] = webhook_url;
                      data["webhook_interval"] = webhook_interval;
                      send_json_response(200, doc); });

        server.on("/api/v1/webhook", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<384> body;
                      if (!parse_json_body(body))
                          return;

                      if (!body["webhook_on"].isNull())
                          webhook_on = body["webhook_on"].as<bool>();
                      if (!body["webhook_url"].isNull())
                          webhook_url = body["webhook_url"].as<String>();
                      if (!body["webhook_interval"].isNull())
                          webhook_interval = body["webhook_interval"].as<int>();

                      send_status_ok(); });

        // Prefix
        server.on("/api/v1/prefix", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<192> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["prefix"] = prefix;
                      send_json_response(200, doc); });

        server.on("/api/v1/prefix", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<256> body;
                      if (!parse_json_body(body))
                          return;

                      if (!body["prefix"].isNull())
                          prefix = body["prefix"].as<String>();

                      send_status_ok(); });

        // Protected inventory
        server.on("/api/v1/protected-inventory", HTTP_GET, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<224> doc;
                      doc["status"] = "ok";
                      JsonObject data = doc.createNestedObject("data");
                      data["enabled"] = protected_inventory_enabled;
                      data["password"] = protected_inventory_password;
                      send_json_response(200, doc); });

        server.on("/api/v1/protected-inventory", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      StaticJsonDocument<320> body;
                      if (!parse_json_body(body))
                          return;

                      if (!body["enabled"].isNull())
                          protected_inventory_enabled = body["enabled"].as<bool>();

                      if (!body["password"].isNull())
                      {
                          String pwd = body["password"].as<String>();
                          pwd.toUpperCase();
                          if (validateHex(pwd, 8) || pwd.length() == 0)
                          {
                              protected_inventory_password = pwd.length() == 0 ? "00000000" : pwd;
                          }
                          else
                          {
                              send_status_error(400, "invalid password, expected 8 hex chars");
                              return;
                          }
                      }

                      reader_module.setup_reader();
                      send_status_ok(); });

        // Firmware update
        server.on("/api/v1/system/update/firmware", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      if (!Update.hasError())
                      {
                          Serial.println("Atualizacao de firmware concluida com sucesso.");
                          send_status_ok();
                      }
                      else
                      {
                          Serial.println("Erro na atualizacao de firmware");
                          Update.printError(Serial);
                          send_status_error(500, "firmware update failed");
                      } }, []()
                  {
                      HTTPUpload &upload = server.upload();

                      if (upload.status == UPLOAD_FILE_START)
                      {
                          Serial.printf("Iniciando upload de firmware: %s\n", upload.filename.c_str());
                          esp_task_wdt_reset();
                          const size_t expected_size = (upload.totalSize > 0) ? upload.totalSize : UPDATE_SIZE_UNKNOWN;
                          if (!Update.begin(expected_size))
                          {
                              Serial.println("Falha ao iniciar Update.begin() para firmware");
                              Update.printError(Serial);
                          }
                      }
                      else if (upload.status == UPLOAD_FILE_WRITE)
                      {
                          esp_task_wdt_reset();
                          yield();
                          if (upload.currentSize && Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                          {
                              Serial.println("Erro ao gravar chunk de firmware");
                              Update.printError(Serial);
                          }
                      }
                      else if (upload.status == UPLOAD_FILE_END)
                      {
                          esp_task_wdt_reset();
                          if (Update.end(true))
                          {
                              Serial.printf("Firmware atualizado: %u bytes\n", upload.totalSize);
                          }
                          else
                          {
                              Serial.println("Erro ao finalizar Update.end() para firmware");
                              Update.printError(Serial);
                          }
                      }
                      else
                      {
                          esp_task_wdt_reset();
                          yield();
                      } });

        // Filesystem update
        server.on("/api/v1/system/update/filesystem", HTTP_POST, []()
                  {
                      if (!ensure_html_route_auth())
                          return;

                      if (!Update.hasError())
                      {
                          send_status_ok();
                      }
                      else
                      {
                          send_status_error(500, "filesystem update failed");
                      } }, []()
                  {
                      HTTPUpload &upload = server.upload();

                      if (upload.status == UPLOAD_FILE_START)
                      {
                          Serial.printf("Recebendo FS: %s\n", upload.filename.c_str());
                          esp_task_wdt_reset();

                          if (fs_loaded)
                          {
                              Serial.println("[update_fs] Desmontando LittleFS antes da escrita");
                              LittleFS.end();
                              fs_loaded = false;
                              delay(10);
                          }

                          const size_t expected_size = (upload.totalSize > 0) ? upload.totalSize : UPDATE_SIZE_UNKNOWN;
                          if (!Update.begin(expected_size, U_SPIFFS, 0x290000))
                          {
                              Serial.println("Falha ao iniciar Update.begin() para FS");
                              Update.printError(Serial);
                              if (!fs_loaded && LittleFS.begin(false))
                              {
                                  fs_loaded = true;
                                  Serial.println("[update_fs] LittleFS remontado apos falha em Update.begin()");
                              }
                          }
                      }
                      else if (upload.status == UPLOAD_FILE_WRITE)
                      {
                          esp_task_wdt_reset();
                          yield();
                          if (upload.currentSize && Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                          {
                              Serial.println("Erro ao gravar chunk do FS");
                              Update.printError(Serial);
                          }
                      }
                      else if (upload.status == UPLOAD_FILE_END)
                      {
                          esp_task_wdt_reset();
                          if (Update.end(true))
                          {
                              Serial.println("Atualizacao do FS concluida");
                          }
                          else
                          {
                              Serial.println("Erro ao finalizar Update.end() para FS");
                              Update.printError(Serial);
                              if (!fs_loaded && LittleFS.begin(false))
                              {
                                  fs_loaded = true;
                                  Serial.println("[update_fs] LittleFS remontado apos falha no Update.end()");
                              }
                          }
                      }
                      else
                      {
                          esp_task_wdt_reset();
                          yield();
                      } });
    }

    void loop()
    {
        server.handleClient();

        // Reconecta ao Wi-Fi em modo Station se a conexão cair
        if (wifi_ssid.length() > 0 && !wifi_connected)
        {
            static unsigned long last_reconnect = 0;
            const unsigned long reconnect_interval = 30000; // 30 segundos
            if (millis() - last_reconnect > reconnect_interval)
            {
                last_reconnect = millis();
                Serial.println("[wifi] Tentando reconectar ao Wi-Fi...");
                WiFi.reconnect();
            }
        }
    }
};