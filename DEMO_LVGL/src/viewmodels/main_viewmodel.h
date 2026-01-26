#pragma once

#include <functional>
#include "main_ui_state.h"
#include "../services/serial_service.h"

class MainViewModel
{
private:
    SerialService *_serialService;
    MainUiState _uiState;
    std::function<void(const MainUiState &)> _onStateChanged;
    SemaphoreHandle_t _stateMutex; // Protección de datos

public:
    MainViewModel(SerialService *serialService);
    ~MainViewModel();

    void bind(std::function<void(const MainUiState &)> observer);
    void setDeviceConnected(bool isConnected);
    void setConnectionState(ConnectionState state);

private:
    void _handleRawSerial(const SerialEvent &event);
    void _notifyStateChanged();
};