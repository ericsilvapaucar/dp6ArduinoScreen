#pragma once

#include "Arduino.h"
#include "TM1637TinyDisplay6.h"

#define CLK_A   15
#define DIO_A   7

class DisplayService {
    private:
        TM1637TinyDisplay6 display;
    public:

        DisplayService(): display(CLK_A, DIO_A) {}

        void init();

        void showString(const char *str);
};