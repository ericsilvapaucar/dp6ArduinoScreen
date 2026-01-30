#include "UIHelper.h"
#include <lvgl.h>
#include <stdio.h>

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
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_left(container, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_right(container, 16, LV_PART_MAIN);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        container,
        LV_FLEX_ALIGN_CENTER, // eje vertical (uno debajo del otro)
        LV_FLEX_ALIGN_CENTER, // eje horizontal (CENTRADO)
        LV_FLEX_ALIGN_CENTER);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);

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

lv_obj_t *UIHelper::createTotalAmountPanel(lv_obj_t *parent)
{

    lv_obj_t *totalPanel = UIHelper::createPanel(parent);
    lv_obj_align(totalPanel, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_pad_left(totalPanel, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_right(totalPanel, 16, LV_PART_MAIN);

    lv_obj_set_width(totalPanel, lv_pct(100));
    lv_obj_set_height(totalPanel, 65);
    lv_obj_set_flex_flow(totalPanel, LV_FLEX_FLOW_ROW);
    // Equivalente a Arrangement.SpaceBetween y Alignment.CenterVertically
    lv_obj_set_flex_align(totalPanel,
                          LV_FLEX_ALIGN_SPACE_BETWEEN, // Main Axis (Horizontal)
                          LV_FLEX_ALIGN_CENTER,        // Cross Axis (Vertical)
                          LV_FLEX_ALIGN_CENTER);       // Track cross place

    lv_obj_set_style_bg_color(totalPanel, lv_color_hex(0x1FB8B9), LV_PART_MAIN);

    lv_obj_t *totalLabel = lv_label_create(totalPanel);
    lv_label_set_text(totalLabel, "TOTAL");
    lv_obj_set_style_text_color(totalLabel, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(totalLabel, &lv_font_montserrat_24, LV_PART_MAIN);

    lv_obj_t *totalValue = lv_label_create(totalPanel);
    lv_label_set_text(totalValue, "S/ 0.00");
    lv_obj_set_style_text_color(totalValue, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(totalValue, &lv_font_montserrat_24, LV_PART_MAIN);

    return totalValue;
}

lv_obj_t *UIHelper::createProductItem(lv_obj_t *parent, const char* name, const char* quantity, const char* unitName)
{

    lv_obj_t *productItem = UIHelper::createPanel(parent, 12);
    lv_obj_set_layout(productItem, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(productItem, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *label1 = lv_label_create(productItem);
    lv_obj_set_width(label1, lv_pct(100));
    lv_label_set_text(label1, name);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_DOT); // Pone "..." si es largo
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_16, 0); // Un poco más grande
    lv_obj_set_style_text_color(label1, lv_color_white(), 0);

    lv_obj_t *label2 = lv_label_create(productItem);
    char resultado[40];
    snprintf(resultado, sizeof(resultado), "%s %s", quantity, unitName);

    lv_label_set_text(label2, resultado);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label2, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);

    return productItem;

}