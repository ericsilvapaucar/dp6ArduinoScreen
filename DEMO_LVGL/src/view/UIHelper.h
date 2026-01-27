#pragma once
#include <lvgl.h>

extern "C"
{
    LV_IMG_DECLARE(img_casamarket);
    LV_IMG_DECLARE(icon_alert);
    LV_IMG_DECLARE(icon_error);
    LV_IMG_DECLARE(icon_check);
}

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

}