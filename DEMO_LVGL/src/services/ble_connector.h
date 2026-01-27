#pragma once

#include <functional>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "i_bluetooth_connector.h"

#define MAX_BUFFER_SIZE 1024
#define BLE_SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define BLE_CHAR_TX_UUID "12345678-1234-5678-1234-56789abcdef1"
#define BLE_CHAR_RX_UUID "12345678-1234-5678-1234-56789abcdef2"

class BLEConnector : public IBluetoothConnector, BLEServerCallbacks, BLECharacteristicCallbacks
{
private:
    BLEServer *pServer;
    BLECharacteristic *pTxCharacteristic;
    std::function<void(bool)> _onConnectCallback;
    uint8_t bufferGlobal[MAX_BUFFER_SIZE];
    size_t bytesRecibidos = 0;
    uint8_t countChunks = 0;

public:
    bool start() override;
    void send(const std::vector<uint8_t> &data) override;

    void bindConnectionHandler(std::function<void(bool)> handler);

    bool isConnected() override;
    // BLEServerCallbacks
    void onConnect(BLEServer *pServer) override;
    void onDisconnect(BLEServer *pServer) override;
    // BLECharacteristicCallbacks
    void onWrite(BLECharacteristic *pCharacteristic) override;
};