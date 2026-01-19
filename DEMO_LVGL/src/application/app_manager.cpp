#include "app_manager.h"


void AppManager::initialize() {
    // Inicialización de la aplicación

    xTaskCreate(
        AppManager::startTaskImpl,
        "AppManagerTask",
        4096,
        NULL,
        1,
        NULL
    );

}

// Esta es la función "puente" o trampolín
void AppManager::startTaskImpl(void* _this) {
    // Convertimos el void* de vuelta a AppManager*
    AppManager* pAppManager = static_cast<AppManager*>(_this);
    
    // Llamamos al método real usando el puntero recuperado
    pAppManager->run(); 
}

void AppManager::run() {
    for (;;) {
        // Lógica principal de la aplicación

        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 seg

    }
    vTaskDelete(NULL);
}
