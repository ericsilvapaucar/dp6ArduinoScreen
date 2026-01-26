#include "main_screen.h"
#include <lvgl.h>

extern "C"
{
    LV_IMG_DECLARE(img_casamarket);
    LV_IMG_DECLARE(icon_alert);
    LV_IMG_DECLARE(icon_error);
    LV_IMG_DECLARE(icon_check);
}

namespace
{

    lv_obj_t *_createSpinner(lv_obj_t *parent)
    {
        lv_obj_t *spinner = lv_spinner_create(parent, 1000, 60);
        lv_obj_set_size(spinner, 50, 50);
        lv_obj_center(spinner);
        int width = 5;

        // Aplicar al fondo (el círculo completo)
        lv_obj_set_style_arc_width(spinner, width, LV_PART_MAIN);
        // Aplicar al indicador (la parte que gira)
        lv_obj_set_style_arc_width(spinner, width, LV_PART_INDICATOR);
        lv_obj_set_style_arc_rounded(spinner, true, LV_PART_INDICATOR);

        return spinner;
    }

    lv_obj_t *_createCheckIcon(lv_obj_t *parent)
    {
        lv_obj_t *checkIcon = lv_img_create(parent);
        lv_img_set_src(checkIcon, &icon_check);
        lv_obj_center(checkIcon);
        return checkIcon;
    }

    lv_obj_t *_createErrorIcon(lv_obj_t *parent)
    {
        lv_obj_t *errorIcon = lv_img_create(parent);
        lv_img_set_src(errorIcon, &icon_error);
        lv_obj_center(errorIcon);
        return errorIcon;
    }

    lv_obj_t *_createAlertIcon(lv_obj_t *parent)
    {
        lv_obj_t *alertIcon = lv_img_create(parent);
        lv_img_set_src(alertIcon, &icon_alert);
        lv_obj_center(alertIcon);
        return alertIcon;
    }

    lv_obj_t *_createHeaderImage(lv_obj_t *parent)
    {
        lv_obj_t *image_logo = lv_img_create(parent);
        lv_img_set_src(image_logo, &img_casamarket);
        lv_obj_center(image_logo);
        lv_obj_set_style_pad_bottom(image_logo, 24, LV_PART_MAIN);
        return image_logo;
    }

    lv_obj_t *_createFooterLabel(lv_obj_t *parent)
    {
        lv_obj_t* labelStatus = lv_label_create(parent);
        lv_label_set_text(labelStatus, "Esperando conexion...");
        lv_obj_align(labelStatus, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_pad_top(labelStatus, 8, LV_PART_MAIN);
        return labelStatus;
    }
}

void MainScreen::init()
{
    setupUI();
    bindViewModel();
}

void MainScreen::bindViewModel()
{
    LV_LOG_USER("bindViewModel");
    _viewModel->bind([this](const MainUiState &state)
                     {
        _latestState = state;

        LV_LOG_USER("bindViewModel: State changed, scheduling UI update");
        lv_async_call([](void* arg) {
            MainScreen* self = static_cast<MainScreen*>(arg);
            self->render(self->_latestState);
        }, this); });
}

void MainScreen::setupUI()
{

    // 1. Obtener la pantalla/display actual
    lv_disp_t *dispi = lv_disp_get_default();

    // 2. Definir tus colores de marca
    lv_color_t color_primario = lv_color_hex(0xFF6600);   // Naranja CasaMarket
    lv_color_t color_secundario = lv_color_hex(0x00BCD4); // Turquesa CasaMarket

    // 3. Inicializar el tema por defecto con tus colores
    // Parámetros: display, color_primario, color_secundario, modo_oscuro, fuente
    lv_theme_t *th = lv_theme_default_init(dispi,
                                           color_primario,
                                           color_secundario,
                                           false, // Dark mode (true/false)
                                           &lv_font_montserrat_14);

    // 4. Aplicar el tema globalmente
    lv_disp_set_theme(dispi, th);

    // Obtener la pantalla activa
    lv_obj_t *screen = lv_scr_act();

    // Establecer el color (Ejemplo: Gris oscuro)
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFF00), LV_PART_MAIN);

    // Asegurar que la opacidad sea total (255 = 100% opaco)
    lv_obj_set_style_bg_opa(screen, 255, LV_PART_MAIN);

    lv_obj_t *root = lv_obj_create(NULL);

    lv_obj_t *container = lv_obj_create(root);
    lv_obj_set_height(container, LV_SIZE_CONTENT);
    lv_obj_set_width(container, LV_SIZE_CONTENT);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        container,
        LV_FLEX_ALIGN_START,  // eje vertical (uno debajo del otro)
        LV_FLEX_ALIGN_CENTER, // eje horizontal (CENTRADO)
        LV_FLEX_ALIGN_START);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);

    _createHeaderImage(container);

    /*Create a spinner*/
    _spinnerLoading = _createSpinner(container);

    _iconCheck = _createCheckIcon(container);
    lv_obj_add_flag(_iconCheck, LV_OBJ_FLAG_HIDDEN);

    // _createAlertIcon(container);
    // _createErrorIcon(container);

    _labelStatus = _createFooterLabel(container);

    // Button go to success
    lv_obj_t * label;

    lv_obj_t * btn = lv_btn_create(container);
    lv_obj_add_event_cb(btn, event_success_handler, LV_EVENT_ALL, this);

    label = lv_label_create(btn);
    lv_label_set_text(label, "Go to Success");
    lv_obj_center(label);

    lv_scr_load(root);
}

void MainScreen::event_success_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        // Recuperamos la instancia de la pantalla
        MainScreen *self = static_cast<MainScreen *>(lv_event_get_user_data(e));

        if (self != nullptr && self->_viewModel != nullptr)
        {
            LV_LOG_USER("Botón presionado: invirtiendo estado de conexión");

            self->_viewModel->requestListProducts();
        }
    }
}

void MainScreen::render(const MainUiState &state)
{

    switch (state.connectionState)
    {
    case DISCONNECTED:
        lv_obj_clear_flag(_spinnerLoading, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_iconCheck, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(_labelStatus, "Esperando conexion...");
        break;    
    case CONNECTED:
        lv_obj_add_flag(_spinnerLoading, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_iconCheck, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(_labelStatus, "Conexion establecida!");
        break;
    }

}