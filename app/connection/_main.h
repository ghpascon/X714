#include "pins.h"
#include "vars.h"
#include "eth_callback.h"
#include "telnet.h"

class CONNECTION : public TELNET
{
private:
    bool event_registered = false;
    bool eth_initialized = false;
    bool mdns_started = false;
    unsigned long eth_disconnected_since_ms = 0;
    unsigned long last_eth_recovery_attempt_ms = 0;

    static const uint32_t ETH_RECOVERY_GRACE_MS = 8000;
    static const uint32_t ETH_RECOVERY_INTERVAL_MS = 15000;

    void apply_eth_ip_config()
    {
        // Em DHCP, limpa IP estático e volta ao comportamento dinâmico.
        if (dhcp_on)
        {
            ETH.config(IPAddress(), IPAddress(), IPAddress());
            return;
        }

        if (static_ip.length() == 0 || gateway_ip.length() == 0 || subnet_mask.length() == 0)
            return;

        IPAddress ip;
        IPAddress gateway;
        IPAddress subnet;

        if (ip.fromString(static_ip) && gateway.fromString(gateway_ip) && subnet.fromString(subnet_mask))
        {
            ETH.config(ip, gateway, subnet);
        }
        else
        {
            Serial.println("ETH static config ignored: invalid IP settings");
        }
    }

public:
    void setup()
    {
        // Registra callback uma única vez para evitar múltiplos handlers.
        if (!event_registered)
        {
            WiFi.onEvent(WiFiEvent);
            event_registered = true;
        }

#ifdef ETH_POWER_PIN
        pinMode(ETH_POWER_PIN, OUTPUT);
        digitalWrite(ETH_POWER_PIN, HIGH);
#endif

        // Inicializa o W5500 via SPI apenas na primeira execução.
        if (!eth_initialized)
        {
            if (!ETH.begin(
                    ETH_PHY_W5500, // Tipo do PHY
                    1,             // Endereço PHY (1 é padrão para W5500)
                    ETH_CS_PIN,    // Chip Select
                    ETH_INT_PIN,   // Interrupção
                    ETH_RST_PIN,   // Reset
                    SPI2_HOST,     // Host SPI (ESP32-S3 → SPI3_HOST)
                    ETH_SCLK_PIN,  // Clock (SCK)
                    ETH_MISO_PIN,  // MISO
                    ETH_MOSI_PIN)) // MOSI
            {
                Serial.println("ETH start Failed!");
                config_telnet();
                return;
            }

            eth_initialized = true;
            Serial.println("ETH init OK");
        }

        // Define o hostname e aplica a config de IP sem reinicializar o driver.
        ETH.setHostname(get_esp_name().c_str());
        apply_eth_ip_config();

        if (!mdns_started && MDNS.begin(get_esp_name().c_str()))
        {
            mdns_started = true;
            Serial.println("mDNS responder started");
        }

        // Inicia o servidor Telnet
        config_telnet();
    }

    void loop()
    {
        telnet_loop();

        if (eth_connected)
        {
            eth_disconnected_since_ms = 0;
            return;
        }

        if (!eth_initialized)
            return;

        if (eth_state != "disconnected" && eth_state != "stopped")
            return;

        const unsigned long now = millis();
        if (eth_disconnected_since_ms == 0)
            eth_disconnected_since_ms = now;

        if (now - eth_disconnected_since_ms < ETH_RECOVERY_GRACE_MS)
            return;

        if (now - last_eth_recovery_attempt_ms < ETH_RECOVERY_INTERVAL_MS)
            return;

        last_eth_recovery_attempt_ms = now;
        Serial.println("ETH recovery: reinitializing interface");
        eth_initialized = false;
        setup();
    }
};
