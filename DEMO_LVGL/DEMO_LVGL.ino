
#include <Arduino.h>
#include "src/app.h"

void setup()
{
    String title = "LVGL porting example";

    Serial.begin(115200);
    Serial.println(title + " start");

    Serial.println("Initialize panel device");
    
    init_app();

    Serial.println(title + " end");
}

void loop() {

    lv_timer_handler();
    delay(5);

}
