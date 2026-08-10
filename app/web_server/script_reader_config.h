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
            String json = "{";
            json += "\"simple_send\":\"" + String(simple_send ? 1 : 0) + "\",";
            json += "\"session\":\"" + String(session, DEC) + "\",";
            json += "\"gpi_stop_delay\":\"" + String(gpi_stop_delay) + "\"";
            json += "}";
        server.send(200, "application/json", json); });

    server.on("/table_reader_att", HTTP_GET, []()
              {
        String json = "[";
        json += "[\"simple_send\",\"" + String(simple_send ? 1 : 0) + "\"],";
        json += "[\"SESSION:\",\"" + String(session, DEC) + "\"],";
        json += "[\"GPI STOP DELAY:\",\"" + String(gpi_stop_delay) + "ms\"]";
        json += "]";

    server.send(200, "application/json", json); });
}
