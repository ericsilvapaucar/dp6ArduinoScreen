#include "main_viewmodel.h"
#include "main_ui_state.h"
#include <vector>
#include <cstdint>
#include "../model/model.h"

MainViewModel::MainViewModel(SerialService *serialService, BLEConnector *bleConnector)
    : _serialService(serialService), _bleConnector(bleConnector)
{
    _stateMutex = xSemaphoreCreateMutex();
}

MainViewModel::~MainViewModel()
{
    vSemaphoreDelete(_stateMutex);
}

void MainViewModel::bind(std::function<void(const MainUiState &)> observer)
{
    _onStateChanged = observer;

    _serialService->setup([this](SerialEvent event)
                          { this->_handleRawSerial(event); });

    _bleConnector->start();
    _bleConnector->bindConnectionHandler([this](bool connected)
                                         { this->setConnectionState(connected ? CONNECTED : DISCONNECTED); });

    _bleConnector->setReceiveHandler([this](const BluetoothResponse &response)
                                     { this->_handleBluetoothData(response); });
}

void MainViewModel::setDeviceConnected(bool isConnected)
{
    if (xSemaphoreTake(_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        _uiState.isDeviceConnected = isConnected;
        xSemaphoreGive(_stateMutex);
        _notifyStateChanged();
    }
}

void MainViewModel::setConnectionState(ConnectionState state)
{
    if (xSemaphoreTake(_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        _uiState.connectionState = state;
        xSemaphoreGive(_stateMutex);
        _notifyStateChanged();
    }
}

void MainViewModel::requestListProducts()
{  
    _bleConnector->send(std::vector<uint8_t>{0x4});

}

// Private methods

void MainViewModel::_handleRawSerial(const SerialEvent &event)
{
    char barcode[32];
    size_t len = event.len < sizeof(barcode) - 1 ? event.len : sizeof(barcode) - 1;
    memcpy(barcode, event.data, len);
    barcode[len] = '\0';

    Serial.printf("Datos serial recibidos: %s, longitud: %d\n", barcode, event.len);

    std::vector<uint8_t> dataToSend;
    dataToSend.insert(dataToSend.begin(), {'4', ','});
    dataToSend.insert(dataToSend.end(), event.data, event.data + len);

    _bleConnector->send(dataToSend);

}

void MainViewModel::_notifyStateChanged()
{
    if (_onStateChanged)
    {
        _onStateChanged(_uiState);
    }
}

void MainViewModel::_handleBluetoothData(const BluetoothResponse &response)
{
    if (response.len == 0)
    {
        return;
    }

    uint16_t totalItems = response.len / sizeof(Product);

    const Product* products = reinterpret_cast<const Product*>(response.data);

    for (int i = 0; i < totalItems; i++) {
        Serial.printf("Producto %d: %s | Precio: %s\n", i, products[i].name, products[i].price);
        Serial.flush();
    }
}