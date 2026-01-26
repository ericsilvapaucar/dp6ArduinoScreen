#include "ble_connector.h"

void BLEConnector::onWrite(BLECharacteristic *pCharacteristic)
{
    String value = pCharacteristic->getValue().c_str();
    if (value.length() > 0)
    {
        // Aquí es donde la abstracción recibe los datos
        // En un sistema real, podrías pasarlo a una cola de FreeRTOS
        Serial.printf("Datos recibidos: %s\n", value.c_str());
    }
}

void BLEConnector::onConnect(BLEServer *pServer)
{
    if (_onConnectCallback) {
        _onConnectCallback(true);
    }
    Serial.println("Cliente BLE conectado");
}

void BLEConnector::onDisconnect(BLEServer *pServer)
{
    if (_onConnectCallback) {
        _onConnectCallback(false);
    }
    Serial.println("Cliente BLE desconectado");
}

bool BLEConnector::start()
{
    BLEDevice::init("ESP32_Data_Node");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);

    // 1. Crear el Servicio
    BLEService *pService = pServer->createService(BLE_SERVICE_UUID);

    // 2. Característica para ENVIAR datos (Notify/Read)
    pTxCharacteristic = pService->createCharacteristic(
        BLE_CHAR_TX_UUID,
        BLECharacteristic::PROPERTY_NOTIFY);

    pTxCharacteristic->addDescriptor(new BLE2902());

    // 3. Característica para RECIBIR datos (Write)
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        BLE_CHAR_RX_UUID,
        BLECharacteristic::PROPERTY_WRITE);

    pRxCharacteristic->setCallbacks(this);

    pService->start();

    // 4. Iniciar Anuncio (Advertising)
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->start();

    return true;
}

void BLEConnector::bindConnectionHandler(std::function<void(bool)> handler)
{
    _onConnectCallback = handler;
}

void BLEConnector::send(const std::vector<uint8_t> &data)
{
    if (isConnected())
    {
        pTxCharacteristic->setValue(data.data(), data.size());
        pTxCharacteristic->notify(); // Empuja el dato al cliente
    }
}

bool BLEConnector::isConnected()
{
    return pServer->getConnectedCount() > 0;
}