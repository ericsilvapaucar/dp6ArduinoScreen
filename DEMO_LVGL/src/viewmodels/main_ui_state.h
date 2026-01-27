#pragma once

enum ConnectionState {
    DISCONNECTED,
    CONNECTED,
};

struct MainUiState {
    ConnectionState connectionState = DISCONNECTED;
};
