void reader_modes_script()
{
  server.on("/modes_att", HTTP_GET, []()
            {
    const int row = 14;
    const int col = 2;
    const String json_kv[row][col] = {
        {"gpi_start_label", String(start_reading ? "START READING" : (gpi_start ? "ON" : "OFF"))},
        {"gpi_start_color", String(start_reading ? "#cccccc" : (gpi_start ? "#00ff00" : "#ff0000"))},
        {"start_reading_label", String(start_reading ? "ON" : "OFF")},
        {"start_reading_color", String(start_reading ? "#00ff00" : "#ff0000")},
        {"always_send_label", String(always_send ? "ON" : "OFF")},
        {"always_send_color", String(always_send ? "#00ff00" : "#ff0000")},
        {"simple_send_label", String(simple_send ? "ON" : "OFF")},
        {"simple_send_color", String(simple_send ? "#00ff00" : "#ff0000")},
        {"keyboard_label", String(keyboard ? "ON" : "OFF")},
        {"keyboard_color", String(keyboard ? "#00ff00" : "#ff0000")},
        {"buzzer_label", String(buzzer_on ? "ON" : "OFF")},
        {"buzzer_color", String(buzzer_on ? "#00ff00" : "#ff0000")},
        {"gtin_label", String(decode_gtin ? "ON" : "OFF")},
        {"gtin_color", String(decode_gtin ? "#00ff00" : "#ff0000")}
    };

    String json = "{";
    for (int i = 0; i < row; i++) {
      json += "\"" + json_kv[i][0] + "\":\"" + json_kv[i][1] + "\",";
    }
    json += "}";
    json.replace(",}", "}");

    server.send(200, "application/json", json); });

  server.on("/start_reading", HTTP_GET, []()
            { 
        start_reading = !start_reading;
        if (start_reading)
          gpi_start = false;
        
    server.send(200, "application/json", String(start_reading)); });

  server.on("/gpi_start", HTTP_GET, []()
            { 
              if(start_reading)
               {
         server.send(401, "application/json", String("START READ MODE ACTIVE"));
                 return;
                 }
        gpi_start = !gpi_start;
        
    server.send(200, "application/json", String(gpi_start)); });

  server.on("/always_send", HTTP_GET, []()
            { 
        always_send = !always_send;
        
    server.send(200, "application/json", String(always_send)); });

  server.on("/simple_send", HTTP_GET, []()
            { 
        simple_send = !simple_send;
        
    server.send(200, "application/json", String(simple_send)); });

  server.on("/keyboard", HTTP_GET, []()
            { 
        keyboard = !keyboard;
        
    server.send(200, "application/json", String(keyboard)); });

  server.on("/buzzer", HTTP_GET, []()
            { 
        buzzer_on = !buzzer_on;
        
    server.send(200, "application/json", String(buzzer_on)); });

  server.on("/gtin", HTTP_GET, []()
            { 
        decode_gtin = !decode_gtin;
        
    server.send(200, "application/json", String(decode_gtin)); });
}
