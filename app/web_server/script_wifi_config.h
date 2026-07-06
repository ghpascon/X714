void wifi_config_script()
{

    server.on("/get_wifi_config", HTTP_GET, []()
              {
                  // Password is returned for the edit form only (local-network access)
                  String json = "{";
                  json += "\"wifi_ssid\":\"" + wifi_ssid + "\",";
                  json += "\"wifi_password\":\"" + wifi_password + "\"";
                  json += "}";
                  server.send(200, "application/json", json); });

    server.on("/save_wifi_config", HTTP_POST, []()
              {
                  if (server.hasArg("wifi_ssid"))
                  {
                      wifi_ssid = server.arg("wifi_ssid");
                  }
                  if (server.hasArg("wifi_password"))
                  {
                      wifi_password = server.arg("wifi_password");
                  }
                  // Apply new Wi-Fi credentials immediately
                  if (wifi_ssid.length() > 0)
                  {
                      WiFi.disconnect(true);
                      WiFi.mode(WIFI_STA);
                      WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
                  }
                  // Configuration is persisted automatically by save_config() in the main loop
                  server.sendHeader("Location", "/wifi_config");
                  server.send(303); });

    server.on("/table_wifi_att", HTTP_GET, []()
              {
                  String json = "[";
                  json += "[\"SSID\",\"" + wifi_ssid + "\"],";
                  json += "[\"CONNECTION\",\"" + String(wifi_connected ? "CONNECTED" : "DISCONNECTED") + "\"]";
                  if (wifi_connected)
                  {
                      json += ",[\"CURRENT IP\",\"" + WiFi.localIP().toString() + "\"]";
                  }
                  json += "]";
                  server.send(200, "application/json", json); });
}
