
#include <Arduino.h>
#include "src/app.h"

void setup()
{
    String title = "LVGL porting example";

    Serial.begin(115200);
    delay(500);
    Serial.println(title + " start");

    Serial.println("Initialize panel device");
    
    init_app();

    Serial.println(title + " end");
}

void loop() {

    lv_timer_handler();

    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck >= 10000) {
        Serial.printf("RAM: %d KB libres (%.1f%% usado)\n", 
                      ESP.getFreeHeap() / 1024,
                      (1 - ESP.getFreeHeap() / (float)ESP.getHeapSize()) * 100);

        Serial.printf("PSRAM: %d KB libres (%.1f%% usado)\n", 
                      ESP.getFreePsram() / 1024,
                      (1 - ESP.getFreePsram() / (float)ESP.getPsramSize()) * 100);

        lastCheck = millis();
    }

    delay(5);

}
