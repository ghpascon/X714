#include <BLEDevice.h>
#include <BLEUtils.h>
#include <WiFi.h>

// ==================== Forward declarations for connectivity ====================
extern bool eth_connected;
extern bool wifi_connected;
extern bool btConnected;
extern String eth_ip;

// Returns true when any network connection is active.
// When check_bt == false: ETH || WIFI
// When check_bt == true : ETH || WIFI || BT
bool is_connected(bool check_bt = false)
{
    if (check_bt)
        return eth_connected || wifi_connected || btConnected;
    return eth_connected || wifi_connected;
}

String get_connected_ip()
{
    if (eth_connected)
        return eth_ip;
    if (wifi_connected)
        return WiFi.localIP().toString();
    return "";
}

String get_esp_name()
{
    uint64_t chipid = ESP.getEfuseMac();
    char id_str[13];
    sprintf(id_str, "%012llX", chipid);
    return "SMTX-" + String(id_str);
}

String get_bt_mac()
{
    BLEAddress address = BLEDevice::getAddress();
    return String(address.toString().c_str());
}

bool validateHex(String s, int expectedLength)
{
    if (s.length() != expectedLength)
        return false;
    for (unsigned int i = 0; i < s.length(); i++)
    {
        char c = s.charAt(i);
        if (!((c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F')))
            return false;
    }
    return true;
}