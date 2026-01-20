#pragma once

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "i_bluetooth_connector.h"

#define BLE_SERVICE_UUID    "12345678-1234-5678-1234-56789abcdef0"
#define BLE_CHAR_TX_UUID    "12345678-1234-5678-1234-56789abcdef1"
#define BLE_CHAR_RX_UUID    "12345678-1234-5678-1234-56789abcdef2"

class MyBufferCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        String value = pCharacteristic->getValue().c_str();
        if (value.length() > 0) {
            // Aquí es donde la abstracción recibe los datos
            // En un sistema real, podrías pasarlo a una cola de FreeRTOS
            Serial.printf("Datos recibidos: %s\n", value.c_str());
        }
    }
};

class BLEConnector : public IBluetoothConnector {
private:
    BLEServer* pServer;
    BLECharacteristic* pTxCharacteristic;

public:
    bool start() override {
        BLEDevice::init("ESP32_Data_Node");
        pServer = BLEDevice::createServer();
        
        // 1. Crear el Servicio
        BLEService* pService = pServer->createService(BLE_SERVICE_UUID);

        // 2. Característica para ENVIAR datos (Notify/Read)
        pTxCharacteristic = pService->createCharacteristic(
            BLE_CHAR_TX_UUID, 
            BLECharacteristic::PROPERTY_NOTIFY
        );

        pTxCharacteristic->addDescriptor(new BLE2902());


        // 3. Característica para RECIBIR datos (Write)
        BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
            BLE_CHAR_RX_UUID,
            BLECharacteristic::PROPERTY_WRITE
        );

        pRxCharacteristic->setCallbacks(new MyBufferCallbacks());

        pService->start();
        
        // 4. Iniciar Anuncio (Advertising)
        BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
        pAdvertising->start();
        
        return true;
    }

    void send(const std::vector<uint8_t>& data) override {
        if (isConnected()) {
            pTxCharacteristic->setValue(data.data(), data.size());
            pTxCharacteristic->notify(); // Empuja el dato al cliente
        }
    }

    bool isConnected() override {
        return pServer->getConnectedCount() > 0;
    }
};