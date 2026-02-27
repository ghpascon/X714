void webhook_config_script()
{
    server.on("/webhook_config", HTTP_GET, []()
              {
                  File f = LittleFS.open("/html/webhook_config.html", "r");
                  if (!f) { server.send(404, "text/plain", "Not found"); return; }
                  server.streamFile(f, "text/html");
                  f.close(); });

    server.on("/get_webhook_config", HTTP_GET, []()
              {
                  String json = "{";
                  json += "\"webhook_on\":" + String(webhook_on ? "true" : "false") + ",";
                  json += "\"webhook_url\":\"" + webhook_url + "\"";
                  json += "}";
                  server.send(200, "application/json", json); });

    server.on("/save_webhook_config", HTTP_POST, []()
              {
                  if (server.hasArg("webhook_on"))
                  {
                      webhook_on = server.arg("webhook_on") == "1";
                  }
                  if (server.hasArg("webhook_url"))
                  {
                      webhook_url = server.arg("webhook_url");
                  }
                  config_file_commands.save_config();
                  server.sendHeader("Location", "/webhook_config");
                  server.send(303); });

    server.on("/table_webhook_att", HTTP_GET, []()
              {
                  String json = "[";
                  json += "[\"WEBHOOK\",\"" + String(webhook_on ? "ON" : "OFF") + "\"],";
                  json += "[\"URL\",\"" + webhook_url + "\"]";
                  json += "]";
                  server.send(200, "application/json", json); });
}
