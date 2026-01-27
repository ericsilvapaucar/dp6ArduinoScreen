#pragma once

#include <functional>
#include <vector>
#include <cstdint>

struct BluetoothResponse
{
    uint8_t data[1024];
    size_t len;
};

class IBluetoothConnector {
public:
    virtual bool start() = 0;
    virtual void send(const std::vector<uint8_t>& data) = 0;
    virtual bool isConnected() = 0;
    
    // Callback para recibir datos
    void setReceiveHandler(std::function<void(const BluetoothResponse&)> handler) {
        _onReceive = handler;
    }

protected:
    std::function<void(const BluetoothResponse&)> _onReceive;
};