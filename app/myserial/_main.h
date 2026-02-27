#include "vars.h"
#include "ble.h"

class MySerial
{
public:
    void setup()
    {
        Serial.begin(115200);

        setup_bt();

        my_keyboard.begin();
        USB.VID(0x0001);
        USB.PID(0x0001);
        USB.manufacturerName("Smartx");
        USB.productName("XPAD_PLUS");
        USB.usbAttributes(0x80);
        USB.begin();
        my_usb.begin(115200);
    }

    void loop()
    {
        loop_bt();
    }

    void write(const String &data, bool all = false)
    {
        Serial.println(data);

        if (!all && simple_send)
            return;

        if (eth_connected)
            connection.telnet_write(data);
        if (btConnected)
            write_bt(data);

        if (keyboard)
        {
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
        cmd.toLowerCase();
        cmd.replace("  ", " ");

        return cmd;
    }
};
