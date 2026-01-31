#pragma once
#include <functional>
#include <Arduino.h>

enum TypePress
{

    ONE_PRESS,
    TWO_PRESS,
    START_LONG_PRESS,
    END_LONG_PRES

};

class ButtonService
{

private:
    TaskHandle_t _taskHandle = NULL;
    std::function<void(TypePress)> _handler;
    static void taskRead(void *pvParameters);
public:
    ~ButtonService() {
        if (_taskHandle != NULL) {
            vTaskDelete(_taskHandle);
        }
    }

    bool start();

    void bindButtonHandler(std::function<void(TypePress)> handler) {
        _handler = handler;
    }
};