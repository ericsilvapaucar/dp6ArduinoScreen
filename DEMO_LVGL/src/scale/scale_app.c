
/*********************
 *      INCLUDES
 *********************/
#include "scale_app.h"

#if LV_MEM_CUSTOM == 0 && LV_MEM_SIZE < (38ul * 1024ul)
    #error Insufficient memory for lv_demo_widgets. Please set LV_MEM_SIZE to at least 38KB (38ul * 1024ul).  48KB is recommended.
#endif

/*********************
 *      DEFINES
 *********************/

static lv_style_t style_title;

static const lv_font_t * font_large;

void lv_scale_app(void);
void lv_scale_app_close(void);


/**********************
 * IMPLEMENTATION
 **********************/

void lv_scale_app(void) {

    font_large = &lv_font_montserrat_24;

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_obj_t * root = lv_obj_create(lv_scr_act());

    lv_obj_set_style_pad_all(root, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root, 16, LV_PART_MAIN);

    lv_obj_t * label = lv_label_create(root);
    lv_label_set_text(label, "Inicio");
    lv_obj_add_style(label, &style_title, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);


    static lv_coord_t col_dsc[] = {70, 70, 70, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {50, 50, 50, LV_GRID_TEMPLATE_LAST};

    lv_obj_t * grid = lv_obj_create(root);
    lv_obj_set_height(grid, LV_SIZE_CONTENT);
    lv_obj_set_width(grid, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(grid, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(grid, row_dsc, 0);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);


    lv_obj_t * gridLabel;
    lv_obj_t * obj;
    uint32_t i;
    for(i = 0; i < 9; i++) {
        uint8_t col = i % 3;
        uint8_t row = i / 3;

        obj = lv_obj_create(grid);

        lv_obj_set_height(obj, LV_SIZE_CONTENT);
        lv_obj_set_width(obj, LV_SIZE_CONTENT);
        /*Stretch the cell horizontally and vertically too
         *Set span to 1 to make the cell 1 column/row sized*/
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1,
                             LV_GRID_ALIGN_STRETCH, row, 1);

        gridLabel = lv_label_create(obj);
        lv_label_set_text_fmt(gridLabel, "c%d, r%d", col, row);
        lv_obj_center(gridLabel);
    }

}