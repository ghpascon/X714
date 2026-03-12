void config_ant_script()
{
    server.on("/save_parameters", HTTP_POST, []()
              {
        antena[0].active = 0;
        antena[1].active = 0;
        antena[2].active = 0;
        antena[3].active = 0;

        if (server.hasArg("ANT_1_POWER"))
        {
            antena[0].power = (server.arg("ANT_1_POWER")).toInt();
        }
        if (server.hasArg("ANT_2_POWER"))
        {
            antena[1].power = (server.arg("ANT_2_POWER")).toInt();
        }
        if (server.hasArg("ANT_3_POWER"))
        {
            antena[2].power = (server.arg("ANT_3_POWER")).toInt();
        }
        if (server.hasArg("ANT_4_POWER"))
        {
            antena[3].power = (server.arg("ANT_4_POWER")).toInt();
        }

        if (server.hasArg("ANT_1_RSSI"))
        {
            antena[0].rssi = (server.arg("ANT_1_RSSI")).toInt();
        }
        if (server.hasArg("ANT_2_RSSI"))
        {
            antena[1].rssi = (server.arg("ANT_2_RSSI")).toInt();
        }
        if (server.hasArg("ANT_3_RSSI"))
        {
            antena[2].rssi = (server.arg("ANT_3_RSSI")).toInt();
        }
        if (server.hasArg("ANT_4_RSSI"))
        {
            antena[3].rssi = (server.arg("ANT_4_RSSI")).toInt();
        }

        if (server.hasArg("ANT_1_CHECK"))
        {
            antena[0].active = 1;
        }
        if (server.hasArg("ANT_2_CHECK"))
        {
            antena[1].active = 1;
        }
        if (server.hasArg("ANT_3_CHECK"))
        {
            antena[2].active = 1;
        }
        if (server.hasArg("ANT_4_CHECK"))
        {
            antena[3].active = 1;
        }

        for (int i = 0; i < ant_qtd; i++)
        {
            antena[i].rssi = max(antena[i].rssi, min_rssi);
            antena[i].power = constrain(antena[i].power, min_power, max_power);
        }

        reader_module.setup_reader();

        server.send(200, "text/plain", "Dados recebidos com sucesso"); });

    server.on("/get_config", HTTP_GET, []()
              {
        // Example configuration data
        String json = "{";
        for (int i = 0; i < ant_qtd; i++)
        {
            json += "\"ANT_" + String(i + 1) + "_CHECK\":" + String(antena[i].active) + ",";
            json += "\"ANT_" + String(i + 1) + "_POWER\":\"" + String(antena[i].power) + "\",";
            json += "\"ANT_" + String(i + 1) + "_RSSI\":\"" + String(antena[i].rssi) + "\",";
        }
        json += "}";

        json.replace(",}", "}");

        server.send(200, "application/json", json); });

    server.on("/table_att", HTTP_GET, []()
              {
        int row = 4;
        int col = 4;
        String data[row][col] = {
            {"A1", String(antena[0].active ? "ON" : "OFF"), String(antena[0].power), String(antena[0].rssi)},
            {"A2", String(antena[1].active ? "ON" : "OFF"), String(antena[1].power), String(antena[1].rssi)},
            {"A3", String(antena[2].active ? "ON" : "OFF"), String(antena[2].power), String(antena[2].rssi)},
            {"A4", String(antena[3].active ? "ON" : "OFF"), String(antena[3].power), String(antena[3].rssi)}};

        String json = "[";
        for (int i = 0; i < row; i++) {
        if (i > 0) json += ",";
        json += "[";
        for (int j = 0; j < col; j++) {
            if (j > 0) json += ",";
            json += "\"" + data[i][j] + "\"";
        }
        json += "]";
        }
        json += "]";
    server.send(200, "application/json", json); });
}
