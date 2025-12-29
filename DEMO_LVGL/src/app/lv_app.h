/**
 * @file lv_app.h
 *
 */

#ifndef LV_APP_H
#define LV_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_app(void);
void lv_app_close(void);
void lv_app_update_counter(int counter);
void lv_refresh_timer_cb(lv_timer_t * t);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_WIDGETS_H*/
