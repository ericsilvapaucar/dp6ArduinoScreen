#include "lv_app.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>


#if LV_MEM_CUSTOM == 0 && LV_MEM_SIZE < (38ul * 1024ul)
    #error Insufficient memory for lv_demo_widgets. Please set LV_MEM_SIZE to at least 38KB (38ul * 1024ul).  48KB is recommended.
#endif

#define ANIMATION_TIME 300

extern volatile int s_contador_datos;
extern QueueHandle_t sensor_cmd_queue;

static lv_obj_t* s_lbl_counter;

static lv_obj_t* _success_screen;

static lv_obj_t* _main_screen;

static void go_to_success(void);
static void success_create(void);

static void go_to_main(void);
static void main_create(void);

static void event_handler(lv_event_t * e);
static void event_success_handler(lv_event_t * e);
static void event_back_handler(lv_event_t * e);

void lv_app(void) {
    main_create();
    lv_scr_load(_main_screen);
}

static void go_to_main(void) {
    lv_scr_load_anim(_main_screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, ANIMATION_TIME, 0, false);
}

static void go_to_success(void) {
    if (_success_screen == NULL) {
        success_create();
    }
    lv_scr_load_anim(_success_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, ANIMATION_TIME, 0, false);
}

static void main_create(void) {
    _main_screen = lv_obj_create(NULL);

    lv_obj_t * container = lv_obj_create(_main_screen);
    lv_obj_set_height(container, LV_SIZE_CONTENT);
    lv_obj_set_width(container, LV_SIZE_CONTENT);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        container,
        LV_FLEX_ALIGN_START,   // eje vertical (uno debajo del otro)
        LV_FLEX_ALIGN_CENTER,  // eje horizontal (CENTRADO)
        LV_FLEX_ALIGN_START);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * label1 = lv_label_create(container);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center "
                      "and wrap long text automatically.");
    lv_obj_set_width(label1, 250);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);

    s_lbl_counter = lv_label_create(container);
    lv_label_set_text(s_lbl_counter, "0");
    lv_obj_set_width(s_lbl_counter, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(s_lbl_counter, LV_TEXT_ALIGN_CENTER, 0);


    // Button Container
    lv_obj_t * button_container = lv_obj_create(container);
    lv_obj_set_style_pad_all(button_container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(button_container, 0, LV_PART_MAIN);
    
    lv_obj_set_height(button_container, LV_SIZE_CONTENT);
    lv_obj_set_width(button_container, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(button_container, LV_FLEX_FLOW_ROW);

    // Button go to success
    lv_obj_t * label;

    lv_obj_t * btn = lv_btn_create(button_container);
    lv_obj_add_event_cb(btn, event_success_handler, LV_EVENT_ALL, NULL);

    label = lv_label_create(btn);
    lv_label_set_text(label, "Go to Success");
    lv_obj_center(label);


    // Button Reset

    lv_obj_t * btn1 = lv_btn_create(button_container);
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Reset");
    lv_obj_center(label);

    // Button Toggle
    lv_obj_t * btn2 = lv_btn_create(button_container);
    lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);


}

static void success_create(void) {

    _success_screen = lv_obj_create(NULL);
    lv_obj_t * label = lv_label_create(_success_screen);
    lv_label_set_text(label, "Pantalla de Ajustes");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t * btn = lv_btn_create(_success_screen);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_event_cb(btn, event_back_handler, LV_EVENT_ALL, NULL);

    lv_obj_t * back_label = lv_label_create(btn);
    lv_label_set_text(back_label, "Volver");
    lv_obj_center(label);

}

static void event_success_handler(lv_event_t * e) {
    
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        go_to_success();
    }

}

static void event_back_handler(lv_event_t * e) {
    
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        go_to_main();
    }

}




void lv_app_close(void) {

}

void lv_app_update_counter(int counter) {

    lv_label_set_text_fmt(s_lbl_counter, "%d", counter);
}


static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        int cmd = 1;
        xQueueSend(sensor_cmd_queue, &cmd, 0);
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

void lv_refresh_timer_cb(lv_timer_t * t) {

    static int ultimo_valor_mostrado = -1;

    if (s_contador_datos != ultimo_valor_mostrado) {
        ultimo_valor_mostrado = s_contador_datos;
        LV_LOG_USER("Timer %d", s_contador_datos);
        lv_label_set_text_fmt(s_lbl_counter, "%d", s_contador_datos);
    }

}