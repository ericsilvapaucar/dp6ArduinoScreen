#include <stdint.h>

struct __attribute__((packed)) Product {
    char name[24];
    char price[8];
    char total[8];
    char quantity[4];
    
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