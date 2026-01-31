#pragma once
#include <stdint.h>
#include "Audio.h"
#include "../model/model.h"

class VoiceService {
    private:
        Audio audio;
        static void audioTask(void *pvParameters); // Hilo dedicado al audio
    public:
        void setup();
        void play(BeepType beepType);
        void play(VoiceType voiceType, VoiceMessage messageType);
        void setVolume(uint8_t volume);
};
