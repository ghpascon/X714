void reader_script()
{
  server.on("/reader_html_info", HTTP_GET, []()
            {
    const int row = 5;
    const int col = 2;
    const String json_kv[row][col] = {
        {"reading", String(read_on ? "ON" : "OFF")},
        {"temperature", String(temperatura)},
        {"total_tags", "TOTAL: " + String(current_tag)},
        {"read_button_txt", String(gpi_start ? "GPI" : (read_on ? "STOP" : "READ"))},
        {"read_button_color", String(gpi_start ? "#cccccc" : (read_on ? "#ff0000" : "#00ff00"))}};

    String json = "{";
    for (int i = 0; i < row; i++) {
      json += "\"" + json_kv[i][0] + "\":\"" + json_kv[i][1] + "\",";
    }
    json += "}";
    json.replace(",}", "}");

    server.send(200, "application/json", json); });

  server.on("/tags_table_att", HTTP_GET, []()
            {
      // Snapshot com cap para evitar cópias gigantes que causem OOM
      const int SNAPSHOT_LIMIT = 30;
      String json = "[]";

      // Copy up to SNAPSHOT_LIMIT entries into local arrays
      String epc_list[SNAPSHOT_LIMIT];
      String tid_list[SNAPSHOT_LIMIT];
      int ant_list[SNAPSHOT_LIMIT];
      int rssi_list[SNAPSHOT_LIMIT];
      int cnt = 0;
      for (int i = 0; i < max_tags && cnt < SNAPSHOT_LIMIT; i++) {
        if (tags[i].epc == "")
          break;
        epc_list[cnt] = tags[i].epc;
        tid_list[cnt] = tags[i].tid;
        ant_list[cnt] = tags[i].ant_number;
        rssi_list[cnt] = tags[i].rssi;
        cnt++;
      }

      // Build JSON from snapshot
      json = "[";
      for (int i = 0; i < cnt; i++) {
        String epc_display = epc_list[i];
        if (epc_display.length() > 1) {
          epc_display = epc_display.substring(0, epc_display.length() / 2) + "<br>" + epc_display.substring(epc_display.length() / 2);
        }
        String tid_display = tid_list[i];
        if (tid_display.length() > 1) {
          tid_display = tid_display.substring(0, tid_display.length() / 2) + "<br>" + tid_display.substring(tid_display.length() / 2);
        }
        json += "[\"" + String(i + 1) + "\",\"" + epc_display + "\",\"" + tid_display + "\",\"" + String(ant_list[i]) + "\",\"" + String(rssi_list[i]) + "\"],";
      }
      json += "]";
      json.replace(",]", "]");
  server.send(200, "application/json", json); });
}
