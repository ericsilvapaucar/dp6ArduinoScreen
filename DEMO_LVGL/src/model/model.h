#pragma once
#include <stdint.h>

enum class BeepType {
    CONNECTED,
    VOICE,
};

// Definición de los Enums tal cual los pediste
enum class VoiceType : uint8_t {
    STANDARD = 0x00,
    OLD = 0x03,
    FUNNY = 0x01,
    WOMAN = 0x02
};

enum class VoiceMessage : uint8_t{
    ERROR = 0x00,
    NO_STOCK = 0x01,
    PRODUCT_NOT_FOUND = 0x02,
    NO_PRICE = 0x03,
    INVALID = 0x04,
    THANKS = 0x05
};


struct __attribute__((packed)) Product {
    char name[24];
    char price[16];
    char quantity[16];
    char unitName[16];
    uint8_t uuid[16]; 
}; 

// Tamaño total por producto: 44 bytes

enum class TypeMessage : uint8_t {
    ERROR_MESSAGE = 0x00,
    BEEP_MESSAGE = 0x01,
    VOICE_TEST = 0x02,
    PRODUCT_INFO = 0x04,
    SUCCESS_MESSAGE = 0x05,
    STANDBY_MODE = 0x06,
};

struct __attribute__((packed)) ErrorMessage {
    char message[64];
    int8_t code;
    int8_t voiceType;
};