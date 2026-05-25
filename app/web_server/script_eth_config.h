void eth_config_script()
{
    server.on("/eth_config", HTTP_GET, []()
              {
                  bool _fs_locked = false;
                  if (fs_mutex) {
                      if (xSemaphoreTake(fs_mutex, pdMS_TO_TICKS(2000)) == pdTRUE) _fs_locked = true;
                      else { server.send(500, "text/plain", "FS busy"); return; }
                  }
                  File f = LittleFS.open("/html/eth_config.html", "r");
                  if (!f) { if (_fs_locked) xSemaphoreGive(fs_mutex); server.send(404, "text/plain", "Not found"); return; }
                  server.streamFile(f, "text/html");
                  f.close(); if (_fs_locked) xSemaphoreGive(fs_mutex); });

    server.on("/get_eth_config", HTTP_GET, []()
              {
        String json = "{";
        json += "\"dhcp_on\":" + String(dhcp_on ? "true" : "false") + ",";
        json += "\"static_ip\":\"" + static_ip + "\",";
        json += "\"gateway_ip\":\"" + gateway_ip + "\",";
        json += "\"subnet_mask\":\"" + subnet_mask + "\"";
        json += "}";
        server.send(200, "application/json", json); });

    server.on("/save_eth_config", HTTP_POST, []()
              {
        if (server.hasArg("dhcp_on")) {
            dhcp_on = server.arg("dhcp_on") == "1";
        }
        
        if (!dhcp_on) {
            if (server.hasArg("static_ip")) {
                static_ip = server.arg("static_ip");
            }
            if (server.hasArg("gateway_ip")) {
                gateway_ip = server.arg("gateway_ip");
            }
            if (server.hasArg("subnet_mask")) {
                subnet_mask = server.arg("subnet_mask");
            }
        }
        
        connection.setup(); // Reinicia a conexão com as novas configurações
        server.sendHeader("Location", "/eth_config");
        server.send(303); });

    server.on("/table_eth_att", HTTP_GET, []()
              {
        String json = "[[\"DHCP\",\"" + String(dhcp_on ? "ON" : "OFF") + "\"],";
        json += "[\"STATIC IP\",\"" + static_ip + "\"],";
        json += "[\"GATEWAY IP\",\"" + gateway_ip + "\"],";
        json += "[\"SUBNET MASK\",\"" + subnet_mask + "\"],";
        if (eth_connected) {
            json += "[\"CONNECTION\",\"CONNECTED\"],";
            IPAddress ip = ETH.localIP();
            json += "[\"CURRENT IP\",\"" + ip.toString() + "\"]";
        } else {
            json += "[\"CONNECTION\",\"DISCONNECTED\"],";
            json += "[\"CURRENT IP\",\"-\"]";
        }
        json += "]";
        server.send(200, "application/json", json); });
}