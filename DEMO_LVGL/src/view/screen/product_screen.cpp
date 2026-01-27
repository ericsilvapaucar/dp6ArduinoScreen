#include "product_screen.h"
#include "../UIHelper.h"

void ProductScreen::init()
{
    setupUI();
    render();
}

void ProductScreen::setupUI()
{
    // Implementar la configuración de la UI para la pantalla de producto

    UIHelper::applyTheme();

    lv_obj_t *root = UIHelper::createRoot();
    lv_obj_t *container = UIHelper::createMainContainer(root);
    lv_obj_set_width(container, lv_pct(100));
    lv_obj_set_style_pad_left(container, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_right(container, 16, LV_PART_MAIN);

    auto headerImage = UIHelper::createHeaderImage(container);
    lv_img_set_zoom(headerImage, 160);

    lv_obj_t *productItem = UIHelper::createPanel(container, 12);

    lv_obj_set_width(productItem, lv_pct(100));
    lv_obj_set_height(productItem, 72);
    lv_obj_set_flex_flow(productItem, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(productItem, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_bg_color(productItem, lv_color_hex(0xFC894A), LV_PART_MAIN);

    lv_obj_t *label1 = lv_label_create(productItem);
    lv_label_set_text(label1, "TOTAL");

    lv_obj_t *label2 = lv_label_create(productItem);
    lv_label_set_text(label2, "$10.00");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *productItem1 = UIHelper::createPanel(container, 12);

    lv_obj_set_width(productItem1, lv_pct(100));
    lv_obj_set_height(productItem1, 72);
    lv_obj_set_flex_flow(productItem1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(productItem1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_bg_color(productItem1, lv_color_hex(0xE2EDF8), LV_PART_MAIN);



    lv_obj_t *totalPanel = UIHelper::createPanel(container);

    lv_obj_set_width(totalPanel, lv_pct(100));
    lv_obj_set_height(totalPanel, 65);
    lv_obj_set_flex_flow(totalPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(totalPanel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_bg_color(totalPanel, lv_color_hex(0x1FB8B9), LV_PART_MAIN);

    lv_obj_t *totalLabel = lv_label_create(totalPanel);
    lv_label_set_text(totalLabel, "TOTAL");

    lv_obj_t *totalValue = lv_label_create(totalPanel);
    lv_label_set_text(totalValue, "$10.00");
    lv_obj_align(totalPanel, LV_ALIGN_CENTER, 0, 0);

    lv_scr_load(root);
    
}

void ProductScreen::render()
{

}
