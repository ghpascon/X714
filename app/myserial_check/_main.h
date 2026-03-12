#include "vars.h"
#include "serial_write_commands.h"

class MySerialCheck : private serial_port_write
{
public:
    void loop()
    {
        // Lê o input da serial
        String cmd = myserial.check_serial();

        if (cmd.length() > 0)
        {
            // Se for um set_cmd, envia tudo de uma vez
            if (cmd.startsWith("#set_cmd:"))
            {
                check_commands(cmd);
                return; // não continua separando
            }

            int startIndex = 0;
            int separatorIndex = cmd.indexOf('#');
            bool last_cmd = false;
            // Loop para processar todos os comandos separados por '#'
            while (true)
            {
                separatorIndex = cmd.indexOf('#', startIndex + 1);
                if (separatorIndex == -1)
                {
                    separatorIndex = cmd.length();
                    last_cmd = true;
                }
                check_commands(cmd.substring(startIndex, separatorIndex)); // Processa o comando
                startIndex = separatorIndex;
                if (last_cmd)
                    break;
            }
        }
    }

private:
    void change_password_cmd(String cmd)
    {
        if (!cmd.startsWith("#change_password:"))
        {
            myserial.write("#ERROR:Invalid command prefix");
            return;
        }

        String payload = cmd.substring(17); // remove "#change_password:"

        // Split by ';' into max 3 parts: EPC;NEW_PASSWORD;OLD_PASSWORD
        String parts[3];
        int count = 0;
        int start = 0;
        for (int i = 0; i < payload.length() && count < 3; i++)
        {
            if (payload.charAt(i) == ';')
            {
                parts[count++] = payload.substring(start, i);
                start = i + 1;
            }
        }
        parts[count++] = payload.substring(start); // last part

        // Validate number of parts (2 or 3)
        if (count < 2 || count > 3)
        {
            myserial.write("#ERROR:Invalid parameters count");
            return;
        }

        String epc = parts[0];
        String new_password = parts[1];
        String old_password = (count == 3) ? parts[2] : "00000000";

        // Validate EPC (24 hex chars)
        if (epc.length() != 24 || !validateHex(epc, 24))
        {
            myserial.write("#ERROR:Invalid EPC");
            return;
        }

        // Validate new password (8 hex chars)
        if (new_password.length() != 8 || !validateHex(new_password, 8))
        {
            myserial.write("#ERROR:Invalid new password");
            return;
        }

        // Validate old password (8 hex chars)
        if (old_password.length() != 8 || !validateHex(old_password, 8))
        {
            myserial.write("#ERROR:Invalid old password");
            return;
        }

        reader_module.change_password(epc, new_password, old_password);
    }

    void protected_mode_cmd(String cmd)
    {
        if (!cmd.startsWith("#protected_mode:"))
        {
            myserial.write("#ERROR:Invalid command prefix");
            return;
        }

        String payload = cmd.substring(16); // remove "#protected_mode:"

        // Split by ';' into 3 parts: EPC;PASSWORD;ENABLE
        String parts[3];
        int count = 0;
        int start = 0;
        for (int i = 0; i < payload.length() && count < 3; i++)
        {
            if (payload.charAt(i) == ';')
            {
                parts[count++] = payload.substring(start, i);
                start = i + 1;
            }
        }
        parts[count++] = payload.substring(start); // last part

        // Validate number of parts (must be 3)
        if (count != 3)
        {
            myserial.write("#ERROR:Missing parameters");
            return;
        }

        String epc = parts[0];
        String password = parts[1];
        String enable_str = parts[2];
        enable_str.toLowerCase();

        // Validate EPC (24 hex chars)
        if (epc.length() != 24 || !validateHex(epc, 24))
        {
            myserial.write("#ERROR:Invalid EPC");
            return;
        }

        // Validate password (8 hex chars)
        if (password.length() != 8 || !validateHex(password, 8))
        {
            myserial.write("#ERROR:Invalid password");
            return;
        }

        // Validate enable parameter
        if (enable_str != "on" && enable_str != "off" && enable_str != "true" && enable_str != "false")
        {
            myserial.write("#ERROR:Invalid enable parameter");
            return;
        }

        bool enable = (enable_str == "on" || enable_str == "true");
        // mudar a senha antes (usando senha padrão como old_password)
        reader_module.change_password(epc, password);

        reader_module.protected_mode_tag(epc, password, enable);
    }

    void protected_inventory_cmd(String cmd)
    {
        if (!cmd.startsWith("#protected_inventory:"))
        {
            myserial.write("#ERROR:Invalid command prefix");
            return;
        }

        String payload = cmd.substring(21); // remove "#protected_inventory:"

        // Split by ';' into max 2 parts: ENABLE;PASSWORD
        String parts[2];
        int count = 0;
        int start = 0;
        for (int i = 0; i < payload.length() && count < 2; i++)
        {
            if (payload.charAt(i) == ';')
            {
                parts[count++] = payload.substring(start, i);
                start = i + 1;
            }
        }
        parts[count++] = payload.substring(start); // last part

        // Validate number of parts (1 or 2)
        if (count < 1 || count > 2)
        {
            myserial.write("#ERROR:Invalid parameters count");
            return;
        }

        String enable_str = parts[0];
        enable_str.toLowerCase();
        String password = (count == 2) ? parts[1] : "00000000";

        // Validate enable parameter
        if (enable_str != "on" && enable_str != "off" && enable_str != "true" && enable_str != "false")
        {
            myserial.write("#ERROR:Invalid enable parameter");
            return;
        }

        // Password will be validated inside protected_inventory function
        bool enable = (enable_str == "on" || enable_str == "true");

        // Save configuration
        protected_inventory_enabled = enable;
        protected_inventory_password = password;

        // enable + password
        myserial.write(String(enable) + " " + password);

        reader_module.protected_inventory(enable, password);
    }

public:
    void check_commands(String cmd)
    {
        if (cmd.length() == 0)
            return;
        // ================= Comandos Gerais =================
        if (cmd == "#ping" || cmd == "ping")
        {
            myserial.write("#PONG");
        }
        else if (cmd == "#read:on" || cmd == "readtag on")
        {
            read_on = true;
            myserial.write("#READ:" + String(read_on ? "on" : "off"));
        }
        else if (cmd == "#read:off" || cmd == "readtag off")
        {
            read_on = false;
            myserial.write("#READ:" + String(read_on ? "on" : "off"));
        }
        else if (cmd == "#get_tags")
        {
            tag_commands.tag_data_display();
        }
        else if (cmd == "#get_tags_all")
        {
            tag_commands.tag_data_display_all();
        }
        else if (cmd == "#clear")
        {
            tag_commands.clear_tags();
            myserial.write("#TAGS_CLEARED");
        }
        else if (cmd.startsWith("#write:"))
        {
            write_tag_cmd(cmd);
        }
        else if (cmd.startsWith("#change_password:"))
        {
            change_password_cmd(cmd);
        }
        else if (cmd == "#get_state")
        {
            myserial.write(read_on ? "#READING" : "#IDLE");
        }
        else if (cmd == "#get_power")
        {
            myserial.write("#POWER:" + String(antena[0].power));
        }
        else if (cmd == "#get_session")
        {
            myserial.write("#SESSION:" + String(session));
        }
        else if (cmd == "#restart")
        {
            ESP.restart();
        }
        else if (cmd == "#get_serial")
        {
            myserial.write("#SERIAL:" + get_esp_name());
        }
        else if (cmd == "#get_info")
        {
            myserial.write("#NAME:" + get_esp_name());
            myserial.write("#BT_MAC:" + get_bt_mac());
            myserial.write("#ETH_MAC:" + String(ETH.macAddress()));
            myserial.write("#IP:" + ETH.localIP().toString());
        }

        // ================= Comandos de Configuração =================
        else if (cmd.startsWith("#prefix:"))
        {
            String v = cmd.substring(String("#prefix:").length());
            v.trim();
            prefix = v;
            myserial.write("#PREFIX:" + prefix);
        }
        else if (cmd == "#get_prefix")
        {
            myserial.write("#PREFIX:" + prefix);
        }

        // ================= Comandos do Set_cmd migrados =================
        else if (cmd.startsWith("#set_ant:"))
        {
            // Formato: #set_ant:n,active,power,rssi
            String ant_cmd = cmd.substring(9);
            const String sep = ",";
            int idx = ant_cmd.indexOf(sep);
            if (idx == -1)
                return;
            int current_ant = ant_cmd.substring(0, idx).toInt();
            ant_cmd = ant_cmd.substring(idx + 1);
            idx = ant_cmd.indexOf(sep);
            if (idx == -1)
                return;
            String current_active = ant_cmd.substring(0, idx);
            ant_cmd = ant_cmd.substring(idx + 1);
            idx = ant_cmd.indexOf(sep);
            if (idx == -1)
                return;
            byte current_power = ant_cmd.substring(0, idx).toInt();
            ant_cmd = ant_cmd.substring(idx + 1);
            byte current_rssi = ant_cmd.toInt();
            current_power = constrain(current_power, min_power, max_power);
            current_rssi = max(current_rssi, min_rssi);
            antena_commands.set_antena(current_ant, String(current_active == "on"), current_power, current_rssi);
            myserial.write("#ANT_CONFIGURED:" + String(current_ant) + "," + String(current_active == "on" ? "on" : "off") + "," + String(current_power) + "," + String(current_rssi));
        }
        else if (cmd.startsWith("#session:"))
        {
            String val = cmd.substring(9);
            session = val.toInt();
            if (session > max_session)
                session = 0x00;
            myserial.write("#SESSION:" + String(session));
        }
        else if (cmd.startsWith("#read_power:"))
        {
            String val = cmd.substring(12);
            antena_commands.set_power_all(val.toInt());
            myserial.write("#POWER:" + String(val.toInt()));
        }
        else if (cmd.startsWith("#buzzer:"))
        {
            buzzer_on = cmd.endsWith("on");
            myserial.write("#BUZZER:" + String(buzzer_on ? "on" : "off"));
        }
        else if (cmd.startsWith("#gpi_stop_delay:"))
        {
            String val = cmd.substring(16);
            gpi_stop_delay = val.toInt();
        }
        else if (cmd.startsWith("#decode_gtin:"))
        {
            decode_gtin = cmd.endsWith("on");
            myserial.write("#DECODE_GTIN:" + String(decode_gtin ? "on" : "off"));
        }
        else if (cmd.startsWith("#start_reading:"))
        {
            start_reading = cmd.endsWith("on");
            read_on = start_reading;
            myserial.write("#START_READING:" + String(read_on ? "on" : "off"));
        }
        else if (cmd.startsWith("#gpi_start:"))
        {
            gpi_start = cmd.endsWith("on");
            myserial.write("#GPI_START:" + String(gpi_start ? "on" : "off"));
        }
        else if (cmd.startsWith("#always_send:"))
        {
            always_send = cmd.endsWith("on");
            myserial.write("#ALWAYS_SEND:" + String(always_send ? "on" : "off"));
        }
        else if (cmd.startsWith("#simple_send:"))
        {
            simple_send = cmd.endsWith("on");
            myserial.write("#SIMPLE_SEND:" + String(simple_send ? "on" : "off"));
        }
        else if (cmd.startsWith("#keyboard:"))
        {
            keyboard = cmd.endsWith("on");
            myserial.write("#KEYBOARD:" + String(keyboard ? "on" : "off"));
        }

        // ================= Comandos de Modo =================
        else if (cmd.startsWith("readmode"))
        {
            keyboard = cmd.endsWith("hid");
            myserial.write("#READ_MODE:" + String(keyboard ? "HID" : "USB"));
        }
        else if (cmd.startsWith("initreadtag:"))

        {
            start_reading = cmd.endsWith("on");
            read_on = start_reading;
            myserial.write("#INIT_READ_TAG:" + String(read_on ? "on" : "off"));
        }
        else if (cmd.startsWith("readpower:"))
        {
            String val = cmd.substring(10);
            antena_commands.set_power_all(val.toInt());
            myserial.write("#READ_POWER:" + String(val.toInt()));
        }
        else if (cmd.startsWith("gen2session:"))
        {
            String lastCharStr = cmd.substring(cmd.length() - 1);
            session = lastCharStr.toInt();
            if (session > max_session)
                session = 0x00;
            myserial.write("#GEN2_SESSION:" + String(session));
        }

        // ================= Comandos Protegidos =================
        else if (cmd.startsWith("#protected_mode:"))
        {
            protected_mode_cmd(cmd);
        }
        else if (cmd.startsWith("#protected_inventory:"))
        {
            protected_inventory_cmd(cmd);
            myserial.write("#PROTECTED_INVENTORY:" + String(protected_inventory_enabled ? "ENABLED" : "DISABLED"));
        }
        else if (cmd == "#get_protected_inventory")
        {
            myserial.write("#PROTECTED_INVENTORY:" + String(protected_inventory_enabled ? "ENABLED" : "DISABLED"));
            myserial.write("#PROTECTED_INVENTORY_PASSWORD:" + protected_inventory_password);
        }

        // ================= Comando set_cmd =================
        else if (cmd.startsWith("#set_cmd:"))
        {
            String cmds = cmd.substring(9);
            const String sep = "|";
            while (cmds.length() > 0)
            {
                int idx = cmds.indexOf(sep);
                String current_cmd = (idx == -1) ? cmds : cmds.substring(0, idx);
                current_cmd.trim();
                if (current_cmd.length() > 0)
                {
                    String hash_cmd = "#" + current_cmd;
                    check_commands(hash_cmd);
                }
                if (idx == -1)
                    break;
                cmds = cmds.substring(idx + 1);
            }
        }

        // ================= Setup =================
        else if (cmd == "#setup_reader")
        {
            reader_module.setup_reader();
            myserial.write("#SETUP_STARTED");
        }

        else if (cmd.startsWith("#hotspot"))
        {
            hotspot_on = cmd.endsWith("on");
            myserial.write("#HOTSPOT:" + String(hotspot_on ? "ON" : "OFF"));
        }
        // ================= Fallback =================
        else
        {
            myserial.write("#INVALID_CMD: " + cmd);
        }
    }
};
