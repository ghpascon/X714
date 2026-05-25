void prefix_script()
{
    server.on("/prefix", HTTP_GET, []()
              {
                  bool _fs_locked = false;
                  if (fs_mutex) {
                      if (xSemaphoreTake(fs_mutex, pdMS_TO_TICKS(2000)) == pdTRUE) _fs_locked = true;
                      else { server.send(500, "text/plain", "FS busy"); return; }
                  }
                  File f = LittleFS.open("/html/prefix.html", "r");
                  if (!f) { if (_fs_locked) xSemaphoreGive(fs_mutex); server.send(404, "text/plain", "Not found"); return; }
                  server.streamFile(f, "text/html");
                  f.close(); if (_fs_locked) xSemaphoreGive(fs_mutex); });

    server.on("/get_prefix", HTTP_GET, []()
              {
                  String json = "{";
                  json += "\"prefix\":\"" + prefix + "\"";
                  json += "}";
                  server.send(200, "application/json", json); });

    server.on("/save_prefix", HTTP_POST, []()
              {
                  if (server.hasArg("prefix"))
                  {
                      prefix = server.arg("prefix");
                  }
                  server.sendHeader("Location", "/prefix");
                  server.send(303); });

    server.on("/table_prefix_att", HTTP_GET, []()
              {
                  String json = "[";
                  json += "[\"PREFIX\",\"" + prefix + "\"]";
                  json += "]";
                  server.send(200, "application/json", json); });
}
