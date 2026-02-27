void eth_config_script()
{
    server.on("/eth_config", HTTP_GET, []()
              {
                  File f = LittleFS.open("/html/eth_config.html", "r");
                  if (!f) { server.send(404, "text/plain", "Not found"); return; }
                  server.streamFile(f, "text/html");
                  f.close(); });

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
        
        config_file_commands.save_config();
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