#include <BLEDevice.h>
#include <BLEUtils.h>
#include <WiFi.h>

// ==================== Forward declarations for connectivity ====================
extern bool eth_connected;
extern bool wifi_connected;
extern bool btConnected;
// Cached Ethernet IP updated by connection callbacks.
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

static bool isUnsignedNumber(const String &value)
{
    if (value.length() == 0)
        return false;

    unsigned long parsed = 0;
    for (unsigned int i = 0; i < value.length(); i++)
    {
        const char c = value.charAt(i);
        if (c < '0' || c > '9')
            return false;
        parsed = parsed * 10 + (unsigned long)(c - '0');
        if (parsed > 65535UL)
            return false;
    }

    return true;
}

String get_renner_sku(String epc)
{
    if (epc.length() < 16)
        return "";
    String hex_epc = epc.substring(4, 16);
    unsigned long long decimal_value = strtoull(hex_epc.c_str(), NULL, 16);
    return String(decimal_value);
}

String normalize_simple_send_format(const String &raw_format)
{
    String format = raw_format;
    format.trim();
    format.toLowerCase();

    if (format.length() == 0)
        return "";

    if (format == "renner")
        return "renner";

    if (format == "rchlo")
        format = "3;14";

    const int separator_pos = format.indexOf(';');
    if (separator_pos <= 0 || separator_pos >= (int)format.length() - 1)
        return "";

    String start_str = format.substring(0, separator_pos);
    String end_str = format.substring(separator_pos + 1);
    start_str.trim();
    end_str.trim();

    if (!isUnsignedNumber(start_str) || !isUnsignedNumber(end_str))
        return "";

    const int start_idx = start_str.toInt();
    const int end_idx = end_str.toInt();
    if (start_idx < 0 || end_idx <= start_idx)
        return "";

    return String(start_idx) + ";" + String(end_idx);
}

String format_simple_send_epc(const String &epc, const String &raw_format)
{
    String format = raw_format;
    format.trim();

    if (format.length() == 0)
        return epc;

    const String normalized = normalize_simple_send_format(format);
    if (normalized.length() == 0)
        return "";

    if (normalized == "renner")
        return get_renner_sku(epc);

    const int separator_pos = normalized.indexOf(';');
    const int start_idx = normalized.substring(0, separator_pos).toInt();
    const int end_idx = normalized.substring(separator_pos + 1).toInt();

    if (start_idx < 0 || end_idx <= start_idx)
        return "";
    if (end_idx > (int)epc.length())
        return "";

    return epc.substring(start_idx, end_idx);
}

// receive a String and convert it to a byte array
byte *to_bytes(String str)
{
    const int len = str.length() / 2;
    byte *bytes = new byte[len];
    for (int i = 0; i < len; i++)
    {
        String byteStr = str.substring(i * 2, i * 2 + 2);
        bytes[i] = (byte)strtoul(byteStr.c_str(), NULL, 16);
    }
    return bytes;
}
