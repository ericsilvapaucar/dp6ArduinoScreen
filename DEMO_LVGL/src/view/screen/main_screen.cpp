#include "main_screen.h"
#include "../../view/UIHelper.h"

struct DeleteEventParams
{
    MainScreen *context; // Tu clase (this)
    uint8_t uuid[16];    // El ID del producto
    // Constructor para facilitar la vida
    DeleteEventParams(MainScreen* s, const uint8_t* u) : context(s) {
        memcpy(uuid, u, 16); // Copia profunda de los bytes
    }
};

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

    lv_obj_t *listContainer = lv_obj_create(container);
    lv_obj_set_width(listContainer, lv_pct(100));
    lv_obj_set_flex_grow(listContainer, 1);                           // <--- ESTO ES EL Modifier.weight(1f)
    lv_obj_set_flex_flow(listContainer, LV_FLEX_FLOW_COLUMN);         // Los items van en columna
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

            for (int i = 0; i < state.productList.productCount; i++)
            {

                lv_color_t backgroundColor;
                if (i % 2 == 0)
                {
                    backgroundColor = lv_color_hex(0xFC894A);
                }
                else
                {
                    backgroundColor = lv_color_hex(0xE2EDF8);
                }

                lv_color_t contentColor;
                if (i % 2 == 0)
                {
                    contentColor = lv_color_white();
                }
                else
                {
                    contentColor = lv_color_black();
                }

                ProductItemParam params(state.productList.product[i].name, state.productList.product[i].quantity, state.productList.product[i].unitName, backgroundColor, contentColor);

                lv_obj_t *button = UIHelper::createProductItem(_productListContainer, params);

                DeleteEventParams* userData = new DeleteEventParams(this, state.productList.product[i].uuid);
                lv_obj_add_event_cb(button, _onDeleltePressed, LV_EVENT_CLICKED, userData);

                lv_obj_add_event_cb(button, [](lv_event_t *e)
                                            {
                DeleteEventParams* p = (DeleteEventParams*)lv_event_get_user_data(e);
                delete p; }, LV_EVENT_DELETE, userData);
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

void MainScreen::_onDeleltePressed(lv_event_t *e)
{
    LV_LOG_USER("onDeleltePressed");
    lv_event_code_t code = lv_event_get_code(e);
    DeleteEventParams *params = static_cast<DeleteEventParams *>(lv_event_get_user_data(e));
    if (code == LV_EVENT_CLICKED)
    {
        auto uuid = params->uuid;
        params->context->_viewModel->deleteProduct(uuid);
    }
}