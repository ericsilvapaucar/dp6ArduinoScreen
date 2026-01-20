#pragma once

#include "Arduino.h"

#define TXD1 43
#define RXD1 44
#define SERIAL_BUFFER_SIZE 256
#define SERIAL_TIMEOUT_MS 100
#define SERIAL_UART Serial1

class SerialService
{
private: 
    TaskHandle_t _taskHandle;
    uint8_t _buffer[SERIAL_BUFFER_SIZE]; // Buffer estático
public:

    ~SerialService() {

        if (_taskHandle != NULL) {
            vTaskDelete(_taskHandle);
            _taskHandle = NULL;
        }
    }

    bool serial_service_setup()
    {
        SERIAL_UART.begin(9600, SERIAL_8N1, RXD1, TXD1);
        SERIAL_UART.setTimeout(SERIAL_TIMEOUT_MS);

        // Esperar un momento para que el serial se estabilice
        vTaskDelay(pdMS_TO_TICKS(100));

        BaseType_t result = xTaskCreate(
            _serial_loop,
            "SerialReadTask",
            2048,
            this,
            1,
            &_taskHandle);

        if (result != pdPASS) {
            Serial.println("Error: No se pudo crear la tarea serial");
            return false;
        }
        
        Serial.println("SerialService iniciado correctamente");
        return true;
    }

private:
    static void _serial_loop(void *arg)
    {
        SerialService* self = static_cast<SerialService*>(arg);
        Serial.println("SerialReadTask iniciado");
        for (;;)
        {
            if (SERIAL_UART.available())
            {

                // Leer todos los bytes disponibles sin delay
                size_t bytesRead = 0;

                std::vector<uint8_t> buffer;
                while (SERIAL_UART.available() && bytesRead < SERIAL_BUFFER_SIZE)
                {
                    int byte = SERIAL_UART.read();
                    if (byte != -1) {
                        self->_buffer[bytesRead++] = (uint8_t)byte;
                    }

                    vTaskDelay(pdMS_TO_TICKS(5));
                }
                
                // Procesar datos recibidos
                if (bytesRead > 0) {
                    self->_process_data(self->_buffer, bytesRead);
                }
            }

            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
        
    void _process_data(uint8_t* data, size_t length)
    {
        // Usar printf en lugar de String para evitar fragmentación
        Serial.printf("Recibidos %u bytes\n", length);
        
        // Aquí procesas tus datos
        // Por ejemplo, enviar a un ViewModel o Model
        // notifyDataReceived(data, length);
        
        // Si necesitas imprimir los datos:
        // Serial.write(data, length);
    }
};