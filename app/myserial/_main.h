#include "vars.h"
#include "ble.h"

class MySerial
{
public:
    void setup()
    {
        Serial.begin(115200);

        if (hotspot_on)
            setup_bt();

        my_keyboard.begin();
        USB.VID(0x0001);
        USB.PID(0x0001);
        USB.manufacturerName("Smartx");
        USB.productName("X714");
        USB.usbAttributes(0x80);
        USB.begin();
        my_usb.begin(115200);
    }

    void loop()
    {
        connection.loop();
        loop_bt();
        connection_state_changed();
        usb_health_check();
    }

    void stop_bt_for_network(const String &label)
    {
        const bool was_bt_enabled = bt_enabled;
        if (!was_bt_enabled)
            return;

        stop_bt();
        write("BLE stopped due to " + label + " connection");
    }

    void connection_state_changed()
    {
        static String last_eth_state = "";
        static bool last_wifi_connected = false;

        if (eth_state != last_eth_state)
        {
            write("Ethernet state: " + eth_state);
            if (eth_state == "got_ip")
                stop_bt_for_network("Ethernet");
            last_eth_state = eth_state;
        }

        if (wifi_connected != last_wifi_connected)
        {
            write("WiFi state: " + String(wifi_connected ? "connected" : "disconnected"));
            if (wifi_connected)
                stop_bt_for_network("WiFi");
            last_wifi_connected = wifi_connected;
        }
    }

    void write(const String &data, bool all = false)
    {
        Serial.println(data);

        if (!all && simple_send)
            return;

        if (is_connected(false))
            connection.telnet_write(data);
        if (btConnected)
            write_bt(data);

        if (keyboard)
        {
            if (btConnected)
                return;
            int interval = 10;
            for (size_t i = 0; i < data.length(); i++)
            {
                my_keyboard.write(data[i]);
                delay(interval);
                yield();
            }
            my_keyboard.write('\n');
            delay(interval);
            esp_task_wdt_reset();
        }
        else
            my_usb.println(data);
    }

    String readLine(Stream &stream)
    {
        String cmd = "";
        char c;
        while (stream.available())
        {
            c = stream.read();
            if (c == '\r' || c == '\n')
                break;
            cmd += c;
        }
        return cmd;
    }

    String check_serial()
    {
        String cmd = "";

        if (Serial.available())
        {
            cmd = readLine(Serial);
        }
        else if (my_usb.available())
        {
            cmd = readLine(my_usb);
        }
        else if (bt_cmd != "")
        {
            cmd = bt_cmd;
            bt_cmd = "";
        }
        else
        {
            cmd = connection.check_telnet();
        }

        return cmd;
    }

private:
    static constexpr unsigned long USB_HEALTH_INTERVAL_MS = 1000;
    static constexpr uint8_t USB_MAX_RESET_ATTEMPTS = 3;
    static constexpr unsigned long USB_RESET_DELAY_MS = 80;

    unsigned long usb_last_health_check_ms = 0;
    uint8_t usb_reset_attempts = 0;

    void reset_usb_cdc()
    {
        usb_reset_attempts++;
        Serial.println("[USB] CDC reset attempt #" + String(usb_reset_attempts));

        my_usb.end();
        delay(USB_RESET_DELAY_MS);
        my_usb.begin(115200);

        const int status_after_reset = my_usb.available();
        if (status_after_reset >= 0)
        {
            usb_reset_attempts = 0;
            Serial.println("[USB] CDC reset successful");
            return;
        }

        if (usb_reset_attempts >= USB_MAX_RESET_ATTEMPTS)
        {
            Serial.println("[USB] CDC unrecoverable, rebooting ESP");
            delay(200);
            ESP.restart();
        }
    }

    void usb_health_check()
    {
        const unsigned long now = millis();
        if (now - usb_last_health_check_ms < USB_HEALTH_INTERVAL_MS)
            return;
        usb_last_health_check_ms = now;

        const int usb_status = my_usb.available();
        if (usb_status >= 0)
        {
            if (usb_reset_attempts > 0)
            {
                Serial.println("[USB] Health check OK");
                usb_reset_attempts = 0;
            }
            return;
        }

        Serial.println("[USB] Health check error (available < 0)");
        reset_usb_cdc();
    }
};
