#include "serial_service.h"

SerialService::~SerialService()
{

    if (_taskHandle != nullptr)
    {
        vTaskDelete(_taskHandle);
        _taskHandle = nullptr;
    }
}

bool SerialService::setup(std::function<void(const SerialEvent &)> callback)
{
    _onDataCallback = callback;

    SERIAL_UART.begin(9600, SERIAL_8N1, TXD1, RXD1);

    if (!SERIAL_UART) {
        Serial.println("Error: No se pudo inicializar UART");
        return false;
    }

    SERIAL_UART.setTimeout(SERIAL_TIMEOUT_MS);
    SERIAL_UART.setRxBufferSize(512); // Opcional: ajustar buffer

    // Limpiar buffer por si hay basura
    while (SERIAL_UART.available()) {
        SERIAL_UART.read();
    }

    // Esperar un momento para que el serial se estabilice
    vTaskDelay(pdMS_TO_TICKS(100));

    // 4. MEJORA: Calcular stack size dinámicamente según PSRAM
    UBaseType_t stackSize = 2048;
    
    if (psramFound()) {
        // Con PSRAM disponible, podemos ser más generosos
        stackSize = 3072;
        Serial.println("SerialService: Usando stack extendido (PSRAM disponible)");
    }

    BaseType_t result = xTaskCreatePinnedToCore(
        _serial_loop,
        "SerialReadTask",
        stackSize,
        this,
        1,
        &_taskHandle,
        1);

    if (result != pdPASS)
    {
        Serial.println("Error: No se pudo crear la tarea serial");
        return false;
    }

    Serial.println("SerialService iniciado correctamente");
    return true;
}

void SerialService::_serial_loop(void *arg)
{
    SerialService *self = static_cast<SerialService *>(arg);
    Serial.println("SerialReadTask iniciado");
    for (;;)
    {
        if (SERIAL_UART.available())
        {

            // Leer todos los bytes disponibles sin delay
            size_t bytesRead = 0;

            while (SERIAL_UART.available() && bytesRead < SERIAL_BUFFER_SIZE)
            {
                int byte = SERIAL_UART.read();
                if (byte != -1)
                {
                    self->_buffer[bytesRead++] = (uint8_t)byte;
                }

                vTaskDelay(pdMS_TO_TICKS(5));

            }

            // Procesar datos recibidos
            if (bytesRead > 0)
            {
                self->_process_data(self->_buffer, bytesRead);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void SerialService::_process_data(uint8_t *data, size_t length)
{
    SerialEvent event;

    size_t maxLen = sizeof(event.data);
    event.len = (length < maxLen) ? length : maxLen;

    memcpy(event.data, data, length);
    event.len = length;

    if (_onDataCallback)
    {
        _onDataCallback(event);
    }
}