void protected_inventory_script()
{
    server.on("/protected_inventory", HTTP_GET, []()
              {
                  File f = LittleFS.open("/html/protected_inventory.html", "r");
                  if (!f) { server.send(404, "text/plain", "Not found"); return; }
                  server.streamFile(f, "text/html");
                  f.close(); });

    server.on("/get_protected_inventory", HTTP_GET, []()
              {
                  String json = "{";
                  json += "\"enabled\":" + String(protected_inventory_enabled ? "true" : "false") + ",";
                  json += "\"password\":\"" + protected_inventory_password + "\"";
                  json += "}";
                  server.send(200, "application/json", json); });

    server.on("/save_protected_inventory", HTTP_POST, []()
              {
                  if (server.hasArg("enabled"))
                  {
                      protected_inventory_enabled = (server.arg("enabled") == "true");
                  }
                  if (server.hasArg("password"))
                  {
                      String pwd = server.arg("password");
                      pwd.toUpperCase();
                      // Validate password is 8 hex digits
                      if (validateHex(pwd, 8) || pwd.length() == 0)
                      {
                          if (pwd.length() == 0)
                              protected_inventory_password = "00000000";
                          else
                              protected_inventory_password = pwd;
                      }
                  }
                  
                  // Apply configuration immediately
                  reader_module.setup_reader();
                  
                  server.sendHeader("Location", "/protected_inventory");
                  server.send(303); });

    server.on("/table_protected_inventory_att", HTTP_GET, []()
              {
                  String json = "[";
                  json += "[\"ENABLED\",\"" + String(protected_inventory_enabled ? "YES" : "NO") + "\"],";
                  json += "[\"PASSWORD\",\"" + protected_inventory_password + "\"]";
                  json += "]";
                  server.send(200, "application/json", json); });
}