#include "main_viewmodel.h"
#include "main_ui_state.h"
#include <vector>
#include <cstdint>
#include "../model/model.h"

MainViewModel::MainViewModel(SerialService *serialService, BLEConnector *bleConnector, ButtonService *buttonService, VoiceService *voiceService)
    : _serialService(serialService), _bleConnector(bleConnector), _buttonServices(buttonService), _voiceService(voiceService),  _displayService(displayService)
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

    _displayService->init();

    _serialService->setup([this](SerialEvent event)
                          { _handleRawSerial(event); });

    _bleConnector->start();
    _bleConnector->bindConnectionHandler([this](bool connected)
                                         { setConnectionState(connected ? CONNECTED : DISCONNECTED); });

    _bleConnector->setReceiveHandler([this](const BluetoothResponse &response)
                                     { _handleBluetoothData(response); });

    _buttonServices->start();
    _buttonServices->bindButtonHandler([this](TypePress type)
                                       { _handleButtonEvent(type); });

    _voiceService->setup();
}

void MainViewModel::setConnectionState(ConnectionState state)
{
    if (xSemaphoreTake(_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        _uiState.connectionState = state;
        xSemaphoreGive(_stateMutex);
        _notifyStateChanged();
        if (state == CONNECTED) {
        _voiceService->play(BeepType::CONNECTED);
        }
    }
}

void MainViewModel::_setErrorCode(uint8_t errorCode)
{
    if (xSemaphoreTake(_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        _uiState.errorCode = errorCode;
        xSemaphoreGive(_stateMutex);
        _notifyStateChanged();
    }
}

void MainViewModel::_setSuccess(bool success)
{
    if (xSemaphoreTake(_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        _uiState.isSuccess = success;
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

    size_t totalSize = 1 + len;

    std::vector<uint8_t> dataToSend;
    dataToSend.reserve(totalSize);
    dataToSend.push_back(0x04);
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

    TypeMessage type = static_cast<TypeMessage>(response.type);
    Serial.printf("Type message: %d, size: %d\n", type, response.len);
    switch (type)
    {
    case TypeMessage::PRODUCT_INFO:
    {

        size_t headerSize = 16;
        size_t productOffset = headerSize;

        char totalAmount[headerSize];
        memcpy(totalAmount, response.data, sizeof(totalAmount));

        size_t bodySize = response.len - headerSize;
        size_t tSize = sizeof(Product);

        uint16_t totalItems = bodySize / sizeof(Product);
        const Product *products = reinterpret_cast<const Product *>(response.data + productOffset);

        Serial.printf("Size Data: %d Header Size: %d, Body Size: %d, tSize: %d, Total Items: %d\n", response.len, headerSize, bodySize, tSize, totalItems);
        Serial.printf("total amount: %s\n", totalAmount);

        for (int i = 0; i < totalItems; i++)
        {
            Serial.printf("(%d) Nombre: %s | Precio: %s | Cantidad: %s %s\n", i + 1, products[i].name, products[i].price, products[i].quantity, products[i].unitName);
        }
        Serial.flush();
        _setProductList(totalAmount, totalItems, products);

        break;
    }
    case TypeMessage::ERROR_MESSAGE:

        _setErrorCode(response.data[0]);
        sleep(2);
        _setErrorCode(0);
        break;
    case TypeMessage::SUCCESS_MESSAGE:
        _setSuccess(true);
        sleep(2);
        _setSuccess(false);
        break;

    case TypeMessage::BEEP_MESSAGE:
        Serial.println("Beep!");
        break;

    default:
        break;
    }
}

void MainViewModel::_setProductList(const char totalAmount[], uint16_t totalItems, const Product *products)
{
    Serial.printf("ProductList: %s - %d\n", totalAmount, totalItems);
    if (xSemaphoreTake(_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {

        memcpy(_uiState.productList.totalAmount, totalAmount, sizeof(_uiState.productList.totalAmount));
        _uiState.productList.productCount = totalItems;
        memcpy(_uiState.productList.product, products, sizeof(Product) * totalItems);

        xSemaphoreGive(_stateMutex);

        _displayService->showString(_uiState.productList.totalAmount);


        _notifyStateChanged();
    }
}

void MainViewModel::deleteProduct(const u_int8_t uuid[])
{
    // 1. Sabemos que el UUID siempre mide 16 bytes
    const size_t UUID_SIZE = 16;
    const size_t TOTAL_SIZE = 1 + UUID_SIZE; // Comando (1) + UUID (16)

    std::vector<uint8_t> dataToSend;
    dataToSend.reserve(TOTAL_SIZE); // Evitamos reasignaciones de memoria

    // 2. Insertar el comando de eliminar (0x05)
    dataToSend.push_back(0x05);

    // 3. Insertar los 16 bytes del UUID
    // Usamos el tamaño explícito (16) en lugar de sizeof
    dataToSend.insert(dataToSend.end(), uuid, uuid + UUID_SIZE);

    // 4. Enviar a través del conector BLE
    if (_bleConnector)
    {
        _bleConnector->send(dataToSend);
    }
}

void MainViewModel::_handleButtonEvent(TypePress type)
{
    int8_t message;
    switch (type)
    {
    case TypePress::ONE_PRESS:
        message = 0x01;
        break;
    case TypePress::TWO_PRESS:
        message = 0x02;
        break;
    case TypePress::START_LONG_PRESS:
        message = 0x03;
        break;
    case TypePress::END_LONG_PRES:
        message = 0x06;
        break;
    default:
        return;
    }

    Serial.printf("Gesto: %d\n", message);

    _bleConnector->send(std::vector<uint8_t>{message});
}