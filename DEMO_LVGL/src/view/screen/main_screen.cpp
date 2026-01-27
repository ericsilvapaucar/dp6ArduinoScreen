#include "main_screen.h"
#include "../../view/UIHelper.h"


namespace
{

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
    UIHelper::applyTheme();

    lv_obj_t *root = UIHelper::createRoot();
    lv_obj_t *container = UIHelper::createMainContainer(root);

    auto image_logo = UIHelper::createHeaderImage(container);
    lv_obj_center(image_logo);
    lv_obj_set_style_pad_bottom(image_logo, 24, LV_PART_MAIN);

    /*Create a spinner*/
    _spinnerLoading = UIHelper::createSpinner(container);

    _iconCheck = UIHelper::createCheckIcon(container);
    lv_obj_add_flag(_iconCheck, LV_OBJ_FLAG_HIDDEN);

    _labelStatus = _createFooterLabel(container);

    lv_scr_load(root);

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