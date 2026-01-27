#include "UIHelper.h"
#include <lvgl.h>

void UIHelper::applyTheme()
{

    // 1. Obtener la pantalla/display actual
    lv_disp_t *dispi = lv_disp_get_default();

    // Aplicar el fondo a la pantalla actual
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

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
}

lv_obj_t *UIHelper::createRoot()
{
    lv_obj_t *root = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(root, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, LV_PART_MAIN);
    return root;
}

lv_obj_t *UIHelper::createSpinner(lv_obj_t *parent)
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

lv_obj_t *UIHelper::createCheckIcon(lv_obj_t *parent)
{
    lv_obj_t *checkIcon = lv_img_create(parent);
    lv_img_set_src(checkIcon, &icon_check);
    lv_obj_center(checkIcon);
    return checkIcon;
}

lv_obj_t *UIHelper::createErrorIcon(lv_obj_t *parent)
{
    lv_obj_t *errorIcon = lv_img_create(parent);
    lv_img_set_src(errorIcon, &icon_error);
    lv_obj_center(errorIcon);
    return errorIcon;
}

lv_obj_t *UIHelper::createAlertIcon(lv_obj_t *parent)
{
    lv_obj_t *alertIcon = lv_img_create(parent);
    lv_img_set_src(alertIcon, &icon_alert);
    lv_obj_center(alertIcon);
    return alertIcon;
}

lv_obj_t *UIHelper::createHeaderImage(lv_obj_t *parent)
{
    lv_obj_t *image_logo = lv_img_create(parent);
    lv_img_set_src(image_logo, &img_casamarket);
    return image_logo;
}

lv_obj_t *UIHelper::createMainContainer(lv_obj_t *root)
{
    lv_obj_t *container = lv_obj_create(root);
    lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
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

    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
    return container;
}

lv_obj_t *UIHelper::createPanel(lv_obj_t *parent, lv_coord_t radius)
{

    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_style_radius(panel, radius, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    return panel;
}
