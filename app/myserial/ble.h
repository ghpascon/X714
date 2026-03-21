#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer *pServer = nullptr;
BLECharacteristic *pTxCharacteristic = nullptr;
BLEAdvertising *pAdvertising = nullptr;
bool bt_enabled = false;

// ====== HID Keyboard ======
// UUIDs HID
#define BLE_HID_SERVICE_UUID "1812"
#define BLE_HID_REPORT_MAP_UUID "2A4B"
#define BLE_HID_REPORT_UUID "2A4D"
#define BLE_HID_PROTOCOL_MODE_UUID "2A4E"
#define BLE_HID_BOOT_KB_INPUT_UUID "2A22"
#define BLE_HID_INFO_UUID "2A4A"
#define BLE_HID_CTRL_POINT_UUID "2A4C"

BLEService *pHIDService = nullptr;
BLECharacteristic *pHIDInput = nullptr;

// Report Map para teclado padrão (6KRO)
static const uint8_t reportMap[] = {
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)
    0x05, 0x07, //   Usage Page (Key Codes)
    0x19, 0xE0, //   Usage Minimum (224)
    0x29, 0xE7, //   Usage Maximum (231)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x08, //   Report Count (8)
    0x81, 0x02, //   Input (Data, Variable, Absolute)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x08, //   Report Size (8)
    0x81, 0x03, //   Input (Constant)
    0x95, 0x05, //   Report Count (5)
    0x75, 0x01, //   Report Size (1)
    0x05, 0x08, //   Usage Page (LEDs)
    0x19, 0x01, //   Usage Minimum (1)
    0x29, 0x05, //   Usage Maximum (5)
    0x91, 0x02, //   Output (Data, Variable, Absolute)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x03, //   Report Size (3)
    0x91, 0x03, //   Output (Constant)
    0x95, 0x06, //   Report Count (6)
    0x75, 0x08, //   Report Size (8)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x65, //   Logical Maximum (101)
    0x05, 0x07, //   Usage Page (Key Codes)
    0x19, 0x00, //   Usage Minimum (0)
    0x29, 0x65, //   Usage Maximum (101)
    0x81, 0x00, //   Input (Data, Array)
    0xC0        // End Collection
};

String bt_cmd = "";

// ==================== Callbacks ====================
class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        String value = pCharacteristic->getValue();
        if (value.length() > 0)
        {
            bt_cmd = String(value.c_str());
        }
    }
};

class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer) override
    {
        btConnected = true;
        Serial.println("Dispositivo conectado!");
    }

    void onDisconnect(BLEServer *pServer) override
    {
        btConnected = false;
        Serial.println("Dispositivo desconectado!");
        // Reiniciar advertising imediatamente para reconexão
        BLEDevice::startAdvertising();
        Serial.println("Advertising reiniciado após desconexão");
    }
};

// ==================== Setup ====================
void setup_bt()
{
    BLEDevice::init(get_esp_name().c_str());

    // Limpa ponteiros antigos
    pServer = nullptr;
    pTxCharacteristic = nullptr;
    pAdvertising = nullptr;
    pHIDService = nullptr;
    pHIDInput = nullptr;

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // UART BLE
    BLEService *pUARTService = pServer->createService(SERVICE_UUID);
    pTxCharacteristic = pUARTService->createCharacteristic(
        CHARACTERISTIC_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    BLEDescriptor *tx2902 = new BLE2902();
    pTxCharacteristic->addDescriptor(tx2902);
    BLECharacteristic *pRxCharacteristic = pUARTService->createCharacteristic(
        CHARACTERISTIC_RX,
        BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());

    // HID BLE
    pHIDService = pServer->createService(BLE_HID_SERVICE_UUID);
    BLECharacteristic *pReportMap = pHIDService->createCharacteristic(
        BLE_HID_REPORT_MAP_UUID, BLECharacteristic::PROPERTY_READ);
    pReportMap->setValue((uint8_t *)reportMap, sizeof(reportMap));
    pHIDInput = pHIDService->createCharacteristic(
        BLE_HID_REPORT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    BLEDescriptor *hid2902 = new BLE2902();
    pHIDInput->addDescriptor(hid2902);

    // Inicia ambos os serviços
    pUARTService->start();
    pHIDService->start();

    // Advertising
    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->addServiceUUID(BLE_HID_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    bt_enabled = true;
}

// ==================== Loop ====================
void loop_bt()
{
    if (!bt_enabled)
        return;

    const int LOOP_INTERVAL_MS = 500;
    static unsigned long lastLoopTime = millis();
    if (millis() - lastLoopTime < LOOP_INTERVAL_MS)
        return;
    lastLoopTime = millis();

    // Watchdog para evitar travamento
    esp_task_wdt_reset();

    // check_connection
    if (!btConnected && pAdvertising && !pAdvertising->isAdvertising())
    {
        pAdvertising->start();
    }
}

// ==================== Stop ====================
void stop_bt()
{
    if (!bt_enabled)
        return;
    bt_enabled = false; // para o loop_bt imediatamente
    btConnected = false;
    bt_cmd = "";
    delay(100); // aguarda loop_bt sair caso esteja rodando
    if (pAdvertising != nullptr)
    {
        pAdvertising->stop();
        delay(50);
    }
    // Limpa ponteiros e deinit
    BLEDevice::deinit(true);
    pServer = nullptr;
    pTxCharacteristic = nullptr;
    pAdvertising = nullptr;
    pHIDService = nullptr;
    pHIDInput = nullptr;
    Serial.println("BLE desativado (Ethernet conectada)");
}

// ==================== Write ====================
void write_bt(String cmd)
{
    if (!btConnected)
        return;
    if (keyboard)
    {
        if (!cmd.endsWith("\n"))
            cmd += "\n";
        for (size_t i = 0; i < cmd.length(); i++)
        {
            uint8_t report[8] = {0};
            char c = cmd[i];
            // Letras minúsculas
            if (c >= 'a' && c <= 'z')
            {
                report[2] = (c - 'a') + 0x04;
            }
            // Letras maiúsculas
            else if (c >= 'A' && c <= 'Z')
            {
                report[0] = 0x02; // SHIFT
                report[2] = (c - 'A') + 0x04;
            }
            // Números
            else if (c >= '0' && c <= '9')
            {
                // HID: 0x27 = 0, 0x1E = 1 ...
                if (c == '0')
                    report[2] = 0x27;
                else
                    report[2] = (c - '1') + 0x1E;
            }
            // Espaço
            else if (c == ' ')
            {
                report[2] = 0x2C;
            }
            // Enter
            else if (c == '\n' || c == '\r')
            {
                report[2] = 0x28;
            }
            // Hexadecimal (a-f/A-F já cobertos)
            else
            {
                continue;
            }
            if (pHIDInput)
            {
                pHIDInput->setValue(report, 8);
                pHIDInput->notify();
            }
            delay(10);
            // Solta tecla
            memset(report, 0, 8);
            if (pHIDInput)
            {
                pHIDInput->setValue(report, 8);
                pHIDInput->notify();
            }
            delay(5);
        }
    }
    else
    {
        if (pTxCharacteristic)
        {
            pTxCharacteristic->setValue(cmd);
            pTxCharacteristic->notify();
        }
    }
}