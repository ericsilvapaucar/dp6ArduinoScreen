#pragma once
#include "../model/model.h"

#define MAX_PRODUCTS 20 // O el límite que necesite tu negocio

enum ConnectionState {
    DISCONNECTED,
    CONNECTED,
};

struct ProductList {
    char totalAmount[16];
    uint8_t productCount;
    Product product[MAX_PRODUCTS];
};

struct MainUiState {
    ConnectionState connectionState = DISCONNECTED;
    ProductList productList;
    bool isSuccess = false;
    int8_t errorCode = 0;
};

