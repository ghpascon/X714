void wifi_config_script()
{
    server.on("/wifi_config", HTTP_GET, []()
              {
                  if (!ensure_html_route_auth()) return;
                  File f = LittleFS.open("/html/wifi_config.html", "r");
                  if (!f) { server.send(404, "text/plain", "Not found"); return; }
                  server.streamFile(f, "text/html");
                  f.close(); });

    server.on("/get_wifi_config", HTTP_GET, []()
              {
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
