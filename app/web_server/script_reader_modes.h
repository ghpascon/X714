#include <uri/UriBraces.h>

static bool parse_bool_path_value(const String &raw_value, bool &parsed_value)
{
  String value = raw_value;
  value.trim();
  value.toLowerCase();

  if (value == "1" || value == "true" || value == "on")
  {
    parsed_value = true;
    return true;
  }

  if (value == "0" || value == "false" || value == "off")
  {
    parsed_value = false;
    return true;
  }

  return false;
}

static bool parse_non_negative_int_path_value(const String &raw_value, int &parsed_value)
{
  String value = raw_value;
  value.trim();

  if (value.length() == 0)
    return false;

  for (unsigned int i = 0; i < value.length(); i++)
  {
    if (!isDigit(value[i]))
      return false;
  }

  parsed_value = value.toInt();
  return true;
}

static String reader_modes_status_json()
{
  String json = "{";
  json += "\"start_reading\":";
  json += start_reading ? "true" : "false";
  json += ",\"gpi_start\":";
  json += gpi_start ? "true" : "false";
  json += ",\"always_send\":";
  json += always_send ? "true" : "false";
  json += ",\"simple_send\":";
  json += simple_send ? "true" : "false";
  json += ",\"keyboard\":";
  json += keyboard ? "true" : "false";
  json += ",\"buzzer_on\":";
  json += buzzer_on ? "true" : "false";
  json += ",\"decode_gtin\":";
  json += decode_gtin ? "true" : "false";
  json += ",\"indicator_time\":" + String(indicator_time);
  json += "}";
  return json;
}

static void send_reader_modes_status(int http_code = 200)
{
  server.send(http_code, "application/json", reader_modes_status_json());
}

void reader_modes_script()
{
  server.on("/modes_att", HTTP_GET, []()
            { send_reader_modes_status(); });

  server.on(UriBraces("/start_reading/{}"), HTTP_POST, []()
            {
    bool value = false;
    if (!parse_bool_path_value(server.pathArg(0), value))
    {
      server.send(400, "application/json", "{\"error\":\"invalid start_reading value\"}");
      return;
    }

    start_reading = value;
    if (start_reading)
      gpi_start = false;

    send_reader_modes_status(); });

  server.on(UriBraces("/gpi_start/{}"), HTTP_POST, []()
            {
    if (start_reading)
    {
      server.send(409, "application/json", "{\"error\":\"start_reading active\"}");
      return;
    }

    bool value = false;
    if (!parse_bool_path_value(server.pathArg(0), value))
    {
      server.send(400, "application/json", "{\"error\":\"invalid gpi_start value\"}");
      return;
    }

    gpi_start = value;
    send_reader_modes_status(); });

  server.on(UriBraces("/always_send/{}"), HTTP_POST, []()
            {
    bool value = false;
    if (!parse_bool_path_value(server.pathArg(0), value))
    {
      server.send(400, "application/json", "{\"error\":\"invalid always_send value\"}");
      return;
    }

    always_send = value;
    send_reader_modes_status(); });

  server.on(UriBraces("/simple_send/{}"), HTTP_POST, []()
            {
    bool value = false;
    if (!parse_bool_path_value(server.pathArg(0), value))
    {
      server.send(400, "application/json", "{\"error\":\"invalid simple_send value\"}");
      return;
    }

    simple_send = value;
    send_reader_modes_status(); });

  server.on(UriBraces("/keyboard/{}"), HTTP_POST, []()
            {
    bool value = false;
    if (!parse_bool_path_value(server.pathArg(0), value))
    {
      server.send(400, "application/json", "{\"error\":\"invalid keyboard value\"}");
      return;
    }

    keyboard = value;
    send_reader_modes_status(); });

  server.on(UriBraces("/buzzer/{}"), HTTP_POST, []()
            {
    bool value = false;
    if (!parse_bool_path_value(server.pathArg(0), value))
    {
      server.send(400, "application/json", "{\"error\":\"invalid buzzer value\"}");
      return;
    }

    buzzer_on = value;
    send_reader_modes_status(); });

  server.on(UriBraces("/gtin/{}"), HTTP_POST, []()
            {
    bool value = false;
    if (!parse_bool_path_value(server.pathArg(0), value))
    {
      server.send(400, "application/json", "{\"error\":\"invalid gtin value\"}");
      return;
    }

    decode_gtin = value;
    send_reader_modes_status(); });

  server.on(UriBraces("/indicator_time/{}"), HTTP_POST, []()
            {
    int value = 0;
    if (!parse_non_negative_int_path_value(server.pathArg(0), value))
    {
      server.send(400, "application/json", "{\"error\":\"invalid indicator_time value\"}");
      return;
    }

    indicator_time = value;
    send_reader_modes_status(); });
}
