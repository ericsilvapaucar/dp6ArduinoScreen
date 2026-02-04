#pragma once
#include <stdint.h>
#include "Audio.h"
#include "../model/model.h"

class VoiceService {
    private:
        TaskHandle_t _taskHandle = NULL;
        QueueHandle_t _queue = NULL;
        Audio audio;
        SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
        static void audioTask(void *pvParameters); // Hilo dedicado al audio
        void _play(const char filePath[]);
    public:
        ~VoiceService() {
            if (_taskHandle != nullptr) {
                vTaskDelete(_taskHandle);
                _taskHandle = nullptr;
            }

            if (_queue != nullptr) {
                vQueueDelete(_queue);
                _queue = nullptr;
            }
        }
        void setup();
        void play(BeepType beepType);
        void play(VoiceType voiceType, VoiceMessage messageType);
        void setVolume(uint8_t volume);
};
