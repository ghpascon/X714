#include <BLEDevice.h>
#include <BLEUtils.h>

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