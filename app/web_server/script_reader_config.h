void config_reader_script()
{
    server.on("/save_reader_parameters", HTTP_POST, []()
              {
        if (server.hasArg("simple_send")) {
            simple_send = (server.arg("simple_send")).toInt() == 1;
        }

        if (server.hasArg("select_session")) {
            session = (server.arg("select_session")).toInt();
            if (session > max_session)
                session = 0x00;
        }

        if (server.hasArg("gpi_stop_delay"))
        {
            gpi_stop_delay = (server.arg("gpi_stop_delay")).toInt();
        }

        reader_module.setup_reader();
        server.send(200, "text/plain", "Dados salvos com sucesso"); });

    server.on("/get_reader_config", HTTP_GET, []()
              {
            const int row = 3;
            const int col = 2;
            const String json_kv[row][col] = {
                {"simple_send", String(simple_send?1:0)},
                {"session", String(session, DEC)},
                {"gpi_stop_delay", String(gpi_stop_delay)}};

            String json = "{";
            for (int i = 0; i < row; i++) {
            json += "\"" + json_kv[i][0] + "\":\"" + json_kv[i][1] + "\",";
            }
            json += "}";
            json.replace(",}", "}");
        server.send(200, "application/json", json); });

    server.on("/table_reader_att", HTTP_GET, []()
              {
        int row = 3;
        int col = 2;
        String data[row][col] = {
            {"simple_send", String(simple_send ? 1 : 0)},
            {"SESSION:", String(session, DEC)},
            {"GPI STOP DELAY:", String(gpi_stop_delay) + "ms"}};

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
