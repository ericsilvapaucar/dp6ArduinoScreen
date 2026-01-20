#pragma once

#include <functional>
#include "main_ui_state.h"
#include "../services/serial_service.h"

class MainViewModel
{
private:
    SerialService* _serialService;
    MainUiState _uiState;
    std::function<void(const MainUiState &)> _onStateChanged;

public:

    MainViewModel(SerialService* serialService)
     : _serialService(serialService) {}
    

    void bind(std::function<void(const MainUiState &)> observer)
    {
        _onStateChanged = observer;
        _serialService->serial_service_setup();
    }

    void setDeviceConnected(bool isConnected)
    {
        _uiState.isDeviceConnected = isConnected;
        notifyStateChanged();
        
    }

private:
    void notifyStateChanged()
    {
        if (_onStateChanged) {
            _onStateChanged(_uiState);
        }
    }
};