#include "main_screen.h"
#include <lvgl.h>


void MainScreen::bindViewModel() {

    LV_LOG_USER("bindViewModel");
    _viewModel->bind([this](const MainUiState& state) {
        _latestState = state;

        LV_LOG_USER("bindViewModel: State changed, scheduling UI update");
        lv_async_call([](void* arg) {
            MainScreen* self = static_cast<MainScreen*>(arg);
            self->render(self->_latestState);
        }, this);
    });
}

void MainScreen::setupUI() {
    
    lv_obj_t * root = lv_obj_create(NULL);

    lv_obj_t * container = lv_obj_create(root);
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

    _labelStatus = lv_label_create(container);
    lv_label_set_text(_labelStatus, "Sin conexion");
    lv_obj_align(_labelStatus, LV_ALIGN_CENTER, 0, 0);

    
    lv_obj_t * btn = lv_btn_create(container);
    lv_obj_add_event_cb(btn, event_success_handler, LV_EVENT_ALL, this);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Go to Success");
    lv_obj_center(label);

    lv_scr_load(root);

}

void MainScreen::event_success_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        // Recuperamos la instancia de la pantalla
        MainScreen* self = static_cast<MainScreen*>(lv_event_get_user_data(e));
        
        if (self != nullptr && self->_viewModel != nullptr) {
            LV_LOG_USER("Botón presionado: invirtiendo estado de conexión");
            
            // Usamos el estado actual para decidir el nuevo
            bool currentStatus = self->_latestState.isDeviceConnected;
            self->_viewModel->setDeviceConnected(!currentStatus);
        }
    }
}

void MainScreen::render(const MainUiState& state) {
    // Actualizar la UI basada en el estado
    if (state.isDeviceConnected) {
        // Mostrar que el dispositivo está conectado
        lv_label_set_text(_labelStatus, "Conectado");
    } else {
        // Mostrar que el dispositivo no está conectado
        lv_label_set_text(_labelStatus, "Sin conexion");
    }
}