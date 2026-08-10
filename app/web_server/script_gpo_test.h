void gpo_test_script()
{
  server.on("/gpo_att", HTTP_GET, []()
            {
    String json = "{";
    json += "\"gpo_1_bt_label\":\"" + String(gpo[0] ? "ON" : "OFF") + "\",";
    json += "\"gpo_1_bt_color\":\"" + String(gpo[0] ? "#00ff00" : "#ff0000") + "\",";
    json += "\"gpo_2_bt_label\":\"" + String(gpo[1] ? "ON" : "OFF") + "\",";
    json += "\"gpo_2_bt_color\":\"" + String(gpo[1] ? "#00ff00" : "#ff0000") + "\",";
    json += "\"gpo_3_bt_label\":\"" + String(gpo[2] ? "ON" : "OFF") + "\",";
    json += "\"gpo_3_bt_color\":\"" + String(gpo[2] ? "#00ff00" : "#ff0000") + "\"";
    json += "}";

    server.send(200, "application/json", json); });

  server.on("/gpo_1_bt", HTTP_GET, []()
            { 
        gpo[0]=!gpo[0];
    server.send(200, "application/json", String(gpo[0])); });

  server.on("/gpo_2_bt", HTTP_GET, []()
            { 
        gpo[1]=!gpo[1];
    server.send(200, "application/json", String(gpo[1])); });

  server.on("/gpo_3_bt", HTTP_GET, []()
            { 
        gpo[2]=!gpo[2];
    server.send(200, "application/json", String(gpo[2])); });
}
