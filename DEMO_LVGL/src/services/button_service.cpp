#include "button_service.h"

#define PIN_PULSADOR 5

bool ButtonService::start()
{

    // Esperar un momento para que el serial se estabilice
    vTaskDelay(pdMS_TO_TICKS(100));

    // 4. MEJORA: Calcular stack size dinámicamente según PSRAM
    UBaseType_t stackSize = 2048;

    if (psramFound())
    {
        // Con PSRAM disponible, podemos ser más generosos
        stackSize = 3072;
        Serial.println("ButtonService: Usando stack extendido (PSRAM disponible)");
    }

    BaseType_t result = xTaskCreatePinnedToCore(
        taskRead,
        "Core0_Task",
        stackSize,
        this,
        1,
        &_taskHandle,
        0);

    if (result != pdPASS)
    {
        Serial.println("Error: No se pudo crear la tarea serial");
        return false;
    }

    Serial.println("ButtonService iniciado correctamente");
    return true;
}

void ButtonService::taskRead(void *pvParameters)
{

    ButtonService *self = static_cast<ButtonService *>(pvParameters);

    pinMode(PIN_PULSADOR, INPUT_PULLUP);
    // Configuración de tiempos (ms)
    const unsigned long DEBOUNCE_TIME = 50;
    const unsigned long DOUBLE_TAP_GAP = 250;
    const unsigned long LONG_PRESS_TIME = 600;

    // Variables de estado
    bool lastState = true;         // Estado anterior del pin
    unsigned long lastTime = 0;    // Último cambio de estado
    unsigned long pressedTime = 0; // Cuándo se presionó
    int clickCount = 0;            // Contador para doble toque
    bool isLongPressing = false;   // Flag para saber si ya notificamos el inicio del long press

    for (;;)
    {
        bool currentState = !digitalRead(PIN_PULSADOR);

        unsigned long now = millis();

        // 1. Detección de flancos (Presionar / Soltar)
        if (currentState != lastState)
        {
            if (now - lastTime > DEBOUNCE_TIME)
            {
                lastState = currentState;
                lastTime = now;

                if (currentState == LOW)
                {
                    // --- EVENTO: BOTÓN PRESIONADO ---
                    pressedTime = now;
                }
                else
                {
                    // --- EVENTO: BOTÓN SOLTADO ---
                    unsigned long duration = now - pressedTime;

                    if (isLongPressing)
                    {
                        // Se soltó después de un Long Press
                        Serial.println("Gesto: Long Press Release");

                        if (self->_handler)
                        {
                            self->_handler(END_LONG_PRES);
                        }
                        isLongPressing = false;
                        clickCount = 0; // Reset
                    }
                    else if (duration < LONG_PRESS_TIME)
                    {
                        // Fue un clic corto, incrementamos contador para posible doble toque
                        clickCount++;
                    }
                }
            }
        }

        // 2. Lógica de "Long Press Start" (Mientras el botón sigue abajo)
        if (currentState == LOW && !isLongPressing)
        {
            if (now - pressedTime > LONG_PRESS_TIME)
            {
                isLongPressing = true;
                Serial.println("Gesto: Long Press Start");

                if (self->_handler)
                {
                    self->_handler(START_LONG_PRESS);
                }
            }
        }

        // 3. Lógica para diferenciar Un Toque vs Dos Toques (Timeout)
        if (clickCount > 0 && currentState == HIGH)
        {
            // Si ha pasado el tiempo del gap o ya tenemos los dos clics
            if (now - lastTime > DOUBLE_TAP_GAP || clickCount == 2)
            {
                if (clickCount == 1)
                {
                    Serial.println("Gesto: Un Toque");
                    if (self->_handler)
                    {
                        self->_handler(ONE_PRESS);
                    }
                }
                else if (clickCount == 2)
                {
                    Serial.println("Gesto: Dos Toques");
                    if (self->_handler)
                    {
                        self->_handler(TWO_PRESS);
                    }
                }
                clickCount = 0; // Reset contador
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // No saturar la CPU
    }
}