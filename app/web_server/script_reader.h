void reader_script()
{
  server.on("/reader_html_info", HTTP_GET, []()
            {
    const int total = tag_commands.tagCount();
    const int row = 5;
    const int col = 2;
    const String json_kv[row][col] = {
        {"reading",          String(read_on ? "ON" : "OFF")},
        {"temperature",      String(temperatura)},
        {"total_tags",       "TOTAL: " + String(total)},
        {"read_button_txt",  String(gpi_start ? "GPI" : (read_on ? "STOP" : "READ"))},
        {"read_button_color",String(gpi_start ? "#cccccc" : (read_on ? "#ff0000" : "#00ff00"))}};

    String json = "{";
    for (int i = 0; i < row; i++) {
      json += "\"" + json_kv[i][0] + "\":\"" + json_kv[i][1] + "\",";
    }
    json += "}";
    json.replace(",}", "}");

    server.send(200, "application/json", json); });

  server.on("/tags_table_att", HTTP_GET, []()
            {
      const int SNAPSHOT_LIMIT = 30;
      const int total = tag_commands.tagCount();
      const int cnt   = total < SNAPSHOT_LIMIT ? total : SNAPSHOT_LIMIT;

      String json = "[";
      for (int i = 0; i < cnt; i++) {
        const TagRecord* r = tag_commands.getTag(i);
        if (!r) break;

        String epc_display = String(r->epc);
        if (epc_display.length() > 1)
          epc_display = epc_display.substring(0, epc_display.length() / 2) + "<br>" + epc_display.substring(epc_display.length() / 2);

        String tid_display = String(r->tid);
        if (tid_display.length() > 1)
          tid_display = tid_display.substring(0, tid_display.length() / 2) + "<br>" + tid_display.substring(tid_display.length() / 2);

        json += "[\"" + String(i + 1) + "\",\"" + epc_display + "\",\"" + tid_display + "\",\"" + String(r->ant_number) + "\",\"" + String(r->rssi) + "\"],";
      }
      json += "]";
      json.replace(",]", "]");
  server.send(200, "application/json", json); });
}
