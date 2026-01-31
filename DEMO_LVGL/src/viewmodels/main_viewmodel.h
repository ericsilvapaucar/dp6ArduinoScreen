#pragma once

#include <functional>
#include "main_ui_state.h"
#include "../services/serial_service.h"
#include "../services/ble_connector.h"
#include "../services/button_service.h"

class MainViewModel
{
private:
    SerialService *_serialService;
    BLEConnector *_bleConnector;
    ButtonService *_buttonServices;
    MainUiState _uiState;
    std::function<void(const MainUiState &)> _onStateChanged;
    SemaphoreHandle_t _stateMutex; // Protección de datos

public:
    MainViewModel(SerialService *serialService, BLEConnector *bleConnector, ButtonService *ButtonService);
    ~MainViewModel();

    void bind(std::function<void(const MainUiState &)> observer);
    void setConnectionState(ConnectionState state);
    void requestListProducts();
    void deleteProduct(const u_int8_t uuid[]);

private:
    void _setErrorCode(uint8_t errorCode);
    void _setSuccess(bool success);
    void _handleRawSerial(const SerialEvent &event);
    void _notifyStateChanged();
    void _handleBluetoothData(const BluetoothResponse &response);
    void _handleButtonEvent(TypePress type);
    void _setProductList(const char totalAmount[], uint16_t totalItems, const Product* products);
};