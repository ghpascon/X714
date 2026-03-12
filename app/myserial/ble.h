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

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    // TX
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());

    // RX
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_RX,
        BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());

    pService->start();

    // Advertisement
    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
}

// ==================== Loop ====================
void loop_bt()
{
    const int LOOP_INTERVAL_MS = 500;
    static unsigned long lastLoopTime = millis();
    if (millis() - lastLoopTime < LOOP_INTERVAL_MS)
        return;
    lastLoopTime = millis();

    // check_connection
    if (!btConnected && !BLEDevice::getAdvertising()->isAdvertising())
    {
        BLEDevice::startAdvertising();
    }
}

// ==================== Write ====================
void write_bt(String cmd)
{
    if (btConnected)
    {
        pTxCharacteristic->setValue(cmd);
        pTxCharacteristic->notify();
    }
}