#include "ble_connector.h"


void BLEConnector::onWrite(BLECharacteristic *pCharacteristic)
{
    uint8_t* packet = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();
    
    if (length < 4) {
        Serial.println("Error: Paquete demasiado corto");
        bytesRecibidos = 0;
        countChunks = 0;
        return;
    }

    uint8_t type = packet[0];
    uint8_t chunkIdx = packet[1];
    uint8_t totalChunks = packet[2];
    uint8_t offsetSize = packet[3];
    uint8_t* payload = &packet[4];
    size_t payloadLen = length - 4;

    size_t offset = chunkIdx * offsetSize;
    countChunks++;

    Serial.printf("index: %d offset: %d, Datos %d de %d recibidos, longitud del payload: %d, total: %d\n", chunkIdx, offsetSize, countChunks, totalChunks, payloadLen, length);
    // Ir pegando en el buffer
    memcpy(&bufferGlobal[offset], payload, payloadLen);
    bytesRecibidos += payloadLen;

    if (countChunks == totalChunks) {
    
        countChunks = 0;
        BluetoothResponse response;

        response.len = bytesRecibidos;
        bytesRecibidos = 0;

        Serial.printf("Datos %d\n", response.len);

        memcpy(response.data, bufferGlobal, response.len);

        if (_onReceive) {
            _onReceive(response);
        }

    
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
    BLEDevice::startAdvertising();
    
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