#pragma once

#include "Arduino.h"

#define TXD1 17
#define RXD1 18
#define SERIAL_BUFFER_SIZE 256
#define SERIAL_TIMEOUT_MS 100
#define SERIAL_UART Serial1

struct SerialEvent
{
    uint8_t data[32];
    size_t len;
};

class SerialService
{
private:
    TaskHandle_t _taskHandle = NULL;
    uint8_t _buffer[SERIAL_BUFFER_SIZE]; // Buffer estático
    std::function<void(const SerialEvent &)> _onDataCallback;

public:
    ~SerialService();
    bool setup(std::function<void(const SerialEvent&)> callback);

private:
    static void _serial_loop(void *arg);
    void _process_data(uint8_t *data, size_t length);
};