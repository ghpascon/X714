void webhook_config_script()
{
    server.on("/webhook_config", HTTP_GET, []()
              {
                  bool _fs_locked = false;
                  if (fs_mutex) {
                      if (xSemaphoreTake(fs_mutex, pdMS_TO_TICKS(2000)) == pdTRUE) _fs_locked = true;
                      else { server.send(500, "text/plain", "FS busy"); return; }
                  }
                  File f = LittleFS.open("/html/webhook_config.html", "r");
                  if (!f) { if (_fs_locked) xSemaphoreGive(fs_mutex); server.send(404, "text/plain", "Not found"); return; }
                  server.streamFile(f, "text/html");
                  f.close(); if (_fs_locked) xSemaphoreGive(fs_mutex); });

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
