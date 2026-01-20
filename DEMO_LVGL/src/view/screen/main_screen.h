#pragma once

#include <lvgl.h>
#include "../../viewmodels/main_viewmodel.h"

class MainScreen {
    private:
    MainViewModel* _viewModel;
    MainUiState _latestState;
    lv_obj_t* _labelStatus;
public:
    MainScreen(MainViewModel* viewModel) : _viewModel(viewModel) {}

    void init() {
        setupUI();
        bindViewModel();
    }
private:
    void bindViewModel();
    void setupUI();
    void render(const MainUiState& state);
    static void event_success_handler(lv_event_t * e);
};