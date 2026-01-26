#pragma once

enum ConnectionState {
    DISCONNECTED,
    CONNECTED,
};

struct MainUiState {
    bool isDeviceConnected = false;
    char barcode[32];
    ConnectionState connectionState = DISCONNECTED;
};
