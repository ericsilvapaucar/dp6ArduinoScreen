#pragma once

#include <lvgl.h>
#include "../../viewmodels/main_viewmodel.h"

class MainScreen {
    private:
    MainViewModel* _viewModel;
    MainUiState _latestState;
    // MAIN SCREEN UI
    lv_obj_t* _spinnerLoading;
    lv_obj_t* _iconCheck;
    lv_obj_t* _labelStatus;
    // PRODUCT SCREEN UI
    lv_obj_t* _labelTotalAmount;
    lv_obj_t* _productListContainer;

    // SCREENS
    lv_obj_t* _mainScreen;
    lv_obj_t* _productScreen;
public:
    MainScreen(MainViewModel* viewModel) : _viewModel(viewModel) {}
    void init();
private:
    void bindViewModel();
    void setupUI();
    void render(const MainUiState& state);
    void _createMainScreen(lv_obj_t *root);
    void _createProductScreen(lv_obj_t *root);
};