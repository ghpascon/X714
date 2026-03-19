#include <Arduino.h>
#include <WiFi.h>
#include <NetworkClientSecure.h>
#include <HTTPClient.h>

extern bool webhook_on;
extern String webhook_url;
extern bool eth_connected;
extern const int max_tags;
extern TAG tags[];
extern TAG_COMMANDS tag_commands;
extern MySerial myserial;

// Cliente HTTP global reutilizável (HTTPS não deve ser global)
WiFiClient client_http;

// Envia um payload JSON via HTTP ou HTTPS
int send_payload(const String &body)
{
    int httpCode = -1;

    if (webhook_url.length() == 0)
        return httpCode;

    bool is_https = webhook_url.startsWith("https://");
    myserial.write(String("Posting ") + (is_https ? "HTTPS" : "HTTP") + " to " + webhook_url);
    myserial.write("Free heap: " + String(ESP.getFreeHeap()) + " | Max block: " + String(ESP.getMaxAllocHeap()));

    HTTPClient http;

    if (is_https)
    {
        // Client local para cada POST HTTPS (evita connection refused)
        NetworkClientSecure client;
        client.setInsecure(); // ignora SSL (usa eth ou wifi)
        if (http.begin(client, webhook_url))
        {
            http.addHeader("Content-Type", "application/json");
            http.setTimeout(10000);

            httpCode = http.POST(body);

            if (httpCode > 0)
                myserial.write("HTTP code: " + String(httpCode) + " | Response: " + http.getString());
            else
                myserial.write("HTTPS POST failed: " + http.errorToString(httpCode));

            http.end();
        }
        else
        {
            myserial.write("Failed to start HTTPS connection");
        }
    }
    else
    {
        if (http.begin(client_http, webhook_url))
        {
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Connection", "close");
            http.setTimeout(5000);
            http.useHTTP10(true);
            http.setReuse(false);

            httpCode = http.POST(body);

            if (httpCode > 0)
                myserial.write("HTTP code: " + String(httpCode) + " | Response: " + http.getString());
            else
                myserial.write("HTTP POST failed: " + http.errorToString(httpCode));

            http.end();
        }
        else
        {
            myserial.write("Failed to start HTTP connection");
        }
    }

    return httpCode;
}

class WEBHOOK
{
public:
    void setup()
    {
        payload.reserve(256 + batch_size * 128);
    }

    void loop()
    {
        tick();
    }

private:
    String payload;              // buffer JSON reutilizável
    unsigned long last_post = 0; // timestamp da última postagem
    static constexpr int batch_size = 30;

    // Conta quantas tags estão preenchidas
    int count_tags()
    {
        int total = 0;
        for (int i = 0; i < max_tags; i++)
        {
            if (tags[i].epc.length() == 0)
                break;
            total++;
        }
        return total;
    }

    // Loop principal da classe
    void tick()
    {
        const unsigned long period_ms = 10000; // intervalo de envio
        if (!webhook_on || !eth_connected || webhook_url.length() == 0)
            return;
        if (millis() - last_post < period_ms)
            return;
        last_post = millis();

        int total = count_tags();
        if (total <= 0)
        {
            // Keep-alive quando não há tags
            payload = "[{\"device\":\"" + get_esp_name() + "\",\"event_type\":\"keep_alive\",\"event_data\":{}}]";
            (void)send_payload(payload);
            return;
        }

        // Envia em batches
        bool all_ok = true;
        for (int offset = 0; offset < total; offset += batch_size)
        {
            int cnt = min(batch_size, total - offset);
            payload = "[";

            for (int i = 0; i < cnt; i++)
            {
                if (i > 0)
                    payload += ",";
                const TAG &t = tags[offset + i];
                payload += "{\"device\":\"" + get_esp_name() + "\",";
                payload += "\"event_type\":\"tag\",";
                payload += "\"event_data\":{";
                payload += "\"epc\":\"" + t.epc + "\",";
                payload += "\"tid\":\"" + t.tid + "\",";
                payload += "\"ant\":" + String(t.ant_number) + ",";
                payload += "\"rssi\":" + String(t.rssi) + "}}";
            }

            payload += "]";
            myserial.write("Sending batch " + String(offset / batch_size + 1));
            int code = send_payload(payload);
            all_ok = all_ok && (code >= 200 && code < 300);
            delay(10); // evita sobrecarga da lwIP
        }

        if (all_ok)
            tag_commands.clear_tags();
    }
};