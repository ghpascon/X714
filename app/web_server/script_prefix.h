void prefix_script()
{
    server.on("/prefix", HTTP_GET, []()
              {
                  File f = LittleFS.open("/html/prefix.html", "r");
                  if (!f) { server.send(404, "text/plain", "Not found"); return; }
                  server.streamFile(f, "text/html");
                  f.close(); });

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
                  config_file_commands.save_config();
                  server.sendHeader("Location", "/prefix");
                  server.send(303); });

    server.on("/table_prefix_att", HTTP_GET, []()
              {
                  String json = "[";
                  json += "[\"PREFIX\",\"" + prefix + "\"]";
                  json += "]";
                  server.send(200, "application/json", json); });
}
