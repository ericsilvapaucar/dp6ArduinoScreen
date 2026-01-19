#pragma once

#include <Arduino.h>
#include <freeRTOS/semphr.h>
#include "../service/i_bluetooth_connector.h"

class AppManager {
public:
    AppManager(IBluetoothConnector* bleConnector) {
        this->_bluetoothConnector = bleConnector;
    }
    void initialize();
private:
    IBluetoothConnector* _bluetoothConnector;
    void run();
    static void startTaskImpl(void* _this);
};