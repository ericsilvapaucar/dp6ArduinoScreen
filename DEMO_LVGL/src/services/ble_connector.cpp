#include "ble_connector.h"

void BLEConnector::onWrite(BLECharacteristic *pCharacteristic)
{

    Serial.println("Recibiendo datos por BLE");
    uint8_t* packet = pCharacteristic->getData();
    uint8_t length = pCharacteristic->getLength();

    uint8_t chunkIdx = packet[0];
    uint8_t totalChunks = packet[1];
    uint8_t* payload = &packet[2];
    size_t payloadLen = length - 2;

    Serial.printf("Datos %d de %d recibidos, longitud del payload: %d\n", chunkIdx + 1, totalChunks, payloadLen);

    if (chunkIdx == 0) bytesRecibidos = 0; // Reiniciar si es el primero

    // Ir pegando en el buffer
    memcpy(&bufferGlobal[bytesRecibidos], payload, payloadLen);
    bytesRecibidos += payloadLen;

    // ¿Ya llegó el último pedazo?
    if (chunkIdx == totalChunks - 1) {
        
        Serial.println("Datos recibidos completamente por BLE.");

        BluetoothResponse response;

        response.len = payloadLen;

        memcpy(response.data, packet, response.len);

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