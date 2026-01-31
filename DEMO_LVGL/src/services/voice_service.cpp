#include "voice_service.h"
#include "FS.h"
#include "SD_MMC.h"
#include "SPIFFS.h"

static const char *MP3_PATH = "/ZL - Chengdu.mp3"; // Nombre del audio

static const int SD_CLK = 12;
static const int SD_CMD = 11;
static const int SD_D0 = 13;

static const int I2S_BCLK = 42;
static const int I2S_LRC = 2;
static const int I2S_DOUT = 41;

namespace
{

    // Función para obtener el String del VoiceType
    const char *getBeepTypeeName(BeepType type)
    {
        switch (type)
        {
        case BeepType::CONNECTED:
            return "CONNECTED";
        case BeepType::VOICE:
            return "BEEP";
        default:
            return "CONNECTED";
        }
    }

    // Función para obtener el String del VoiceType
    const char *getVoiceTypeName(VoiceType type)
    {
        switch (type)
        {
        case VoiceType::STANDARD:
            return "STANDARD";
        case VoiceType::OLD:
            return "OLD";
        case VoiceType::FUNNY:
            return "FUNNY";
        case VoiceType::WOMAN:
            return "WOMAN";
        default:
            return "STANDARD";
        }
    }

    // Función para obtener el String del VoiceMessage
    const char *getVoiceMessageName(VoiceMessage msg)
    {
        switch (msg)
        {
        case VoiceMessage::ERROR:
            return "ERROR";
        case VoiceMessage::NO_STOCK:
            return "NO_STOCK";
        case VoiceMessage::PRODUCT_NOT_FOUND:
            return "PRODUCT_NOT_FOUND";
        case VoiceMessage::NO_PRICE:
            return "NO_PRICE";
        case VoiceMessage::INVALID:
            return "INVALID";
        case VoiceMessage::THANKS:
            return "THANKS";
        default:
            return "ERROR";
        }
    }
}

void VoiceService::setup()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Error: No se pudo montar el sistema de archivos SPIFFS");
        return;
    }

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(12);

    // Creamos una tarea en el Core 0 (el Core 1 suele ser para la UI/LVGL)
    BaseType_t result = xTaskCreatePinnedToCore(
        this->audioTask,
        "AudioTask",
        10000,
        this, // Pasamos la instancia de la clase como parámetro
        3,    // Prioridad alta para que el audio no tartamudee
        NULL,
        0 // Core 0
    );

    if (result != pdPASS)
    {
        Serial.println("Error: No se pudo crear la tarea serial");
        return;
    }

    Serial.println("VoiceService iniciado correctamente");
}

// El "Worker Thread" del audio
void VoiceService::audioTask(void *pvParameters)
{
    VoiceService *instance = (VoiceService *)pvParameters;
    for (;;)
    {
        instance->audio.loop();       // Alimenta el buffer I2S
        vTaskDelay(pdMS_TO_TICKS(1)); // Cede un poco de tiempo al sistema
    }
}

void VoiceService::play(BeepType beepType)
{

    char filePath[64];

    // Ejemplo de mapeo de archivos en memoria interna (SPIFFS)
    // Estructura: /v[voiceType]_m[messageType].mp3 -> /v0_m1.mp3
    snprintf(filePath, sizeof(filePath), "/%s.mp3", getBeepTypeeName(beepType));

    if (!SPIFFS.exists(filePath))
    {
        Serial.printf("DEBUG: El archivo NO existe en la memoria interna: %s\n", filePath);

        // Opcional: Listar todos los archivos para ver si hay errores de escritura
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file)
        {
            Serial.printf("Archivo en memoria: %s\n", file.name());
            file = root.openNextFile();
        }
    }
    else
    {

        // Si usas SD, sería audio.connecttoFS(SD, filePath);
        // Si usas memoria interna (SPIFFS/LittleFS):
        bool success = audio.connecttoFS(SPIFFS, filePath);
        if (success)
        {
            Serial.printf("Reproduciendo: %s\n", filePath);
        }
        else
        {
            Serial.printf("No se pudo reproducir: %s\n", filePath);
        }
    }
}

void VoiceService::play(VoiceType voiceType, VoiceMessage messageType)
{
    char filePath[64];

    // Ejemplo de mapeo de archivos en memoria interna (SPIFFS)
    // Estructura: /v[voiceType]_m[messageType].mp3 -> /v0_m1.mp3
    snprintf(filePath, sizeof(filePath), "/%s/%s.mp3", getVoiceTypeName(voiceType), getVoiceMessageName(messageType));

    Serial.printf("Reproduciendo: %s\n", filePath);

    // Si usas SD, sería audio.connecttoFS(SD, filePath);
    // Si usas memoria interna (SPIFFS/LittleFS):
    audio.connecttoFS(SPIFFS, filePath);
}

void VoiceService::setVolume(uint8_t volume)
{
    audio.setVolume(volume);
}