#include "vars.h"
#include "script_all.h"
#include "script_ant_config.h"
#include "script_reader_config.h"
#include "script_reader.h"
#include "script_reader_modes.h"
#include "script_gpo_test.h"
#include "script_eth_config.h"
#include "script_webhook_config.h"
#include "script_prefix.h"
#include "script_protected_inventory.h"

class WEB_SERVER
{
public:
    void setup()
    {
        if (!hotspot_on)
            return;
        config_web_server();
        script_web_server();
        if (LittleFS.begin())
        {
            style_web_server();
            routes_web_server();
        }
        server.begin();
    }

    void config_web_server()
    {
        const String ssid = get_esp_name();
        const char *password = "smartx12345";
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP);
        WiFi.setTxPower(WIFI_POWER_19_5dBm);
        WiFi.softAP(ssid.c_str(), password, 6, false, 2);
    }

    void style_web_server()
    {
        server.on("/style.css", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/style.css", "r");
                      if (!f)
                      {
                          server.send(404, "text/plain", "Not found");
                          return;
                      }
                      server.streamFile(f, "text/css");
                      f.close(); });

        server.on("/FONT_SMARTX.woff", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/FONT_SMARTX.woff", "r");
                      if (!f)
                      {
                          server.send(404, "text/plain", "Not found");
                          return;
                      }
                      server.streamFile(f, "font/woff");
                      f.close(); });
    }

    void routes_web_server()
    {
        server.on("/", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/home.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });

        server.on("/reader", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/reader.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });

        server.on("/ant_config", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/ant_config.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });

        server.on("/reader_config", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/reader_config.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });

        server.on("/reader_modes", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/reader_modes.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });

        server.on("/gpo_test", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/gpo_test.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });

        server.on("/eth_config", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/eth_config.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });

        // prefix
        server.on("/prefix", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/prefix.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });

        // protected_inventory
        server.on("/protected_inventory", HTTP_GET, []()
                  {
                      File f = LittleFS.open("/html/protected_inventory.html", "r");
                      if (!f) { server.send(404, "text/plain", "Not found"); return; }
                      server.streamFile(f, "text/html");
                      f.close(); });
    }
    void script_web_server()
    {
        all_script();
        reader_script();
        config_ant_script();
        config_reader_script();
        reader_modes_script();
        gpo_test_script();
        eth_config_script();
        webhook_config_script();
        prefix_script();
        protected_inventory_script();
    }

    void loop()
    {
        server.handleClient();
    }
};