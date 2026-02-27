#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

extern bool webhook_on;
extern String webhook_url;
extern bool eth_connected;
extern const int max_tags;
extern TAG tags[];
extern TAG_COMMANDS tag_commands;

class WEBHOOK
{
public:
    void setup()
    {
        // Reserve once to reduce heap churn/fragmentation
        payload.reserve(256 + batch_size * 128);
    }

    void loop()
    {
        tick();
    }

private:
    String payload;              // reused JSON buffer
    unsigned long last_post = 0; // ms
    static constexpr int batch_size = 50;

    // Compute number of filled tags (contiguous from index 0 until first empty)
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

    // Removed copy arrays: we'll iterate tags[] directly when building the payload

    // Send one JSON payload (HTTP/HTTPS)
    int send_payload(const String &body)
    {
        HTTPClient http;
        int httpCode = -1;
        if (webhook_url.startsWith("https://"))
        {
            WiFiClientSecure client;
            client.setInsecure();
            if (http.begin(client, webhook_url))
            {
                http.setReuse(false);  // avoid keep-alive reuse
                http.useHTTP10(true);  // simpler framing (no chunked)
                http.setTimeout(3000); // ms
                http.addHeader("Content-Type", "application/json");
                http.addHeader("Connection", "close");
                httpCode = http.POST(body);
                http.end();
            }
        }
        else
        {
            WiFiClient client;
            if (http.begin(client, webhook_url))
            {
                http.setReuse(false);
                http.useHTTP10(true);
                http.setTimeout(5000);
                http.addHeader("Content-Type", "application/json");
                http.addHeader("Connection", "close");
                httpCode = http.POST(body);
                http.end();
            }
        }
        return httpCode;
    }

    void tick()
    {
        const unsigned long period_ms = 10000;
        if (!webhook_on)
            return;
        if (!eth_connected)
            return;
        if (webhook_url.length() == 0)
            return;
        if (millis() - last_post < period_ms)
            return;
        last_post = millis();

        // Determine how many tags are present now
        int total = count_tags();
        if (total <= 0)
        {
            // Send keep-alive event as a single-element events array when there are no tags
            payload = "[{\"device\":\"" + get_esp_name() + "\",\"event_type\":\"keep_alive\",\"event_data\":{}}]";
            (void)send_payload(payload);
            return;
        }

        // Post in batches of up to batch_size, building directly from tags[] (no mutex)
        bool all_ok = true;
        for (int offset = 0; offset < total; offset += batch_size)
        {
            int cnt = min(batch_size, total - offset);
            // Build a list (array) of event objects, each with event_type and event_data
            payload = "[";
            for (int i = 0; i < cnt; i++)
            {
                if (i > 0)
                    payload += ",";
                const TAG &t = tags[offset + i];
                payload += "{\"device\":\"" + get_esp_name() + "\","; // event envelope
                payload += "\"event_type\":\"tag\",";
                payload += "\"event_data\":{"; // tag object
                payload += "\"epc\":\"" + t.epc + "\",";
                payload += "\"tid\":\"" + t.tid + "\",";
                payload += "\"ant\":" + String(t.ant_number) + ",";
                payload += "\"rssi\":" + String(t.rssi) + "}}";
            }
            payload += "]";
            int code = send_payload(payload);
            all_ok = all_ok && (code >= 200 && code < 300);
            delay(10); // yield between batches to avoid stressing lwIP
        }

        // If all batches posted successfully, clear tags
        if (all_ok)
        {
            tag_commands.clear_tags();
        }
    }
};
