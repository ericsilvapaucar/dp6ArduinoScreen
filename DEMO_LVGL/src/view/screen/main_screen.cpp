#include "main_screen.h"
#include "../../view/UIHelper.h"

namespace
{

    lv_obj_t *_createFooterLabel(lv_obj_t *parent)
    {
        lv_obj_t *labelStatus = lv_label_create(parent);
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

    _createMainScreen(root);
    _createProductScreen(root);

    lv_scr_load(root);

}

void MainScreen::_createMainScreen(lv_obj_t *root)
{
    lv_obj_t *container = UIHelper::createMainContainer(root);
    auto image_logo = UIHelper::createHeaderImage(container);
    lv_obj_center(image_logo);
    lv_obj_set_style_pad_bottom(image_logo, 24, LV_PART_MAIN);

    /*Create a spinner*/
    _spinnerLoading = UIHelper::createSpinner(container);

    _iconCheck = UIHelper::createCheckIcon(container);
    lv_obj_add_flag(_iconCheck, LV_OBJ_FLAG_HIDDEN);

    _labelStatus = _createFooterLabel(container);

    _mainScreen = container;
}

void MainScreen::_createProductScreen(lv_obj_t *root)
{
    lv_obj_t *container = UIHelper::createMainContainer(root);
    

    auto headerImage = UIHelper::createHeaderImage(container);
    lv_img_set_zoom(headerImage, 160);

    lv_obj_t * listContainer = lv_obj_create(container);
    lv_obj_set_width(listContainer, lv_pct(100));
    lv_obj_set_flex_grow(listContainer, 1); // <--- ESTO ES EL Modifier.weight(1f)
    lv_obj_set_flex_flow(listContainer, LV_FLEX_FLOW_COLUMN); // Los items van en columna
    lv_obj_set_scrollbar_mode(listContainer, LV_SCROLLBAR_MODE_AUTO); // Scroll automático
    lv_obj_set_style_pad_all(listContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(listContainer, 0, LV_PART_MAIN);
    _productListContainer = listContainer;

    _labelTotalAmount = UIHelper::createTotalAmountPanel(container);

    lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);

    _productScreen = container;
}

void MainScreen::render(const MainUiState &state)
{

    switch (state.connectionState)
    {
    case DISCONNECTED:
        // Mostrar pantalla de en espera
        lv_obj_clear_flag(_mainScreen, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_productScreen, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(_spinnerLoading, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_iconCheck, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(_labelStatus, "Esperando conexion...");
        break;
    case CONNECTED:

        if (state.productList.productCount > 0)
        {
            lv_obj_clean(_productListContainer);

            for (int i = 0; i < state.productList.productCount; i++) {

                lv_obj_t *productItem = UIHelper::createProductItem(
                    _productListContainer,
                    state.productList.productt[i].name,
                    state.productList.productt[i].quantity,
                    state.productList.productt[i].unitName);

            }
            
            // Mostrar pantalla de productos
            lv_obj_clear_flag(_productScreen, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_mainScreen, LV_OBJ_FLAG_HIDDEN);

            lv_label_set_text(_labelTotalAmount, state.productList.totalAmount);
            
        }
        else
        {
            // Mostrar pantalla de en espera
            lv_obj_clear_flag(_mainScreen, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_productScreen, LV_OBJ_FLAG_HIDDEN);

            // Mostrar que el dispositivo esta conectado
            lv_obj_add_flag(_spinnerLoading, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(_iconCheck, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(_labelStatus, "Conexion establecida!");
        }

        break;
    }
}