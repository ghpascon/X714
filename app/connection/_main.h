#include "pins.h"
#include "vars.h"
#include "eth_callback.h"
#include "telnet.h"

class CONNECTION : public TELNET
{
public:
    void setup()
    {
        // Registra o callback de eventos Ethernet
        WiFi.onEvent(WiFiEvent);

#ifdef ETH_POWER_PIN
        pinMode(ETH_POWER_PIN, OUTPUT);
        digitalWrite(ETH_POWER_PIN, HIGH);
#endif

        // ===== Inicializa o W5500 via SPI =====
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
        }
        else
        {
            // Define o hostname
            ETH.setHostname(get_esp_name().c_str());

            // Configura IP estático se DHCP estiver desativado e os IPs forem válidos
            if (!dhcp_on && static_ip.length() > 0 && gateway_ip.length() > 0 && subnet_mask.length() > 0)
            {
                IPAddress ip;
                IPAddress gateway;
                IPAddress subnet;

                if (ip.fromString(static_ip) && gateway.fromString(gateway_ip) && subnet.fromString(subnet_mask))
                {
                    ETH.config(ip, gateway, subnet);
                }
            }

            if (MDNS.begin(get_esp_name().c_str()))
            {
                Serial.println("mDNS responder started");
            }
            Serial.println("ETH init OK");
        }

        // Inicia o servidor Telnet
        config_telnet();
    }
};
