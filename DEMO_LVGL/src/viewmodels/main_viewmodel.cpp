#include "main_viewmodel.h"
#include "main_ui_state.h"

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

// Private methods

void MainViewModel::_handleRawSerial(const SerialEvent &event)
{
    if (xSemaphoreTake(_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        size_t len = event.len < sizeof(_uiState.barcode) - 1 ? event.len : sizeof(_uiState.barcode) - 1;
        memcpy(_uiState.barcode, event.data, len);
        _uiState.barcode[len] = '\0'; // Null-terminar la cadena

        xSemaphoreGive(_stateMutex);
        _notifyStateChanged();
    }
}

void MainViewModel::_notifyStateChanged()
{
    if (_onStateChanged)
    {
        _onStateChanged(_uiState);
    }
}