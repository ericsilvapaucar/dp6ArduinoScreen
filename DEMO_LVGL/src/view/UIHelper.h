#pragma once
#include <lvgl.h>

extern "C"
{
    LV_IMG_DECLARE(img_casamarket);
    LV_IMG_DECLARE(icon_alert);
    LV_IMG_DECLARE(icon_error);
    LV_IMG_DECLARE(icon_check);
    LV_IMG_DECLARE(icon_delete);
}

struct ProductItemParam
{
    const char *name;
    const char *quantity;
    const char *unitName;
    lv_color_t backgroundColor;
    lv_color_t textColor;
}; 

namespace UIHelper
{

    void applyTheme();

    lv_obj_t *createRoot();

    lv_obj_t *createSpinner(lv_obj_t *parent);

    lv_obj_t *createCheckIcon(lv_obj_t *parent);

    lv_obj_t *createErrorIcon(lv_obj_t *parent);

    lv_obj_t *createAlertIcon(lv_obj_t *parent);

    lv_obj_t *createHeaderImage(lv_obj_t *parent);

    lv_obj_t *createMainContainer(lv_obj_t *root);

    lv_obj_t *createPanel(lv_obj_t *parent, lv_coord_t radius = 5);

    lv_obj_t *createTotalAmountPanel(lv_obj_t *parent);

    lv_obj_t *createProductItem(lv_obj_t *parent, ProductItemParam param);

    lv_obj_t *createDeleteButton(lv_obj_t *parent, lv_color_t color);

}