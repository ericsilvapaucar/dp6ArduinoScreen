/**
 * @file lv_demos.h
 *
 */

#ifndef APP_H
#define APP_H

#define LVGL_PORT_ROTATION_DEGREE               (0)

#include "drivers/display.h"
#include "drivers/esp_bsp.h"
#include "drivers/lv_port.h"
#include "view/screen/main_screen.h"
#include "view/screen/product_screen.h"

#ifdef __cplusplus
extern "C" {
#endif

// SerialService serialService;
// BLEConnector bleConnector;
// MainViewModel mainViewModel(&serialService, &bleConnector);
// MainScreen mainScreen(&mainViewModel);
ProductScreen productScreen;

void init_app(void) {
    
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
#if LVGL_PORT_ROTATION_DEGREE == 90
        .rotate = LV_DISP_ROT_90,
#elif LVGL_PORT_ROTATION_DEGREE == 270
        .rotate = LV_DISP_ROT_270,
#elif LVGL_PORT_ROTATION_DEGREE == 180
        .rotate = LV_DISP_ROT_180,
#elif LVGL_PORT_ROTATION_DEGREE == 0
        .rotate = LV_DISP_ROT_NONE,
#endif
    };

    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();

    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    bsp_display_lock(0);

    // TODO: Initialize other modules (e.g., BLE, AppManager, etc.)
    productScreen.init();

//     /* Release the mutex */
    bsp_display_unlock();

 }


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_H*/
