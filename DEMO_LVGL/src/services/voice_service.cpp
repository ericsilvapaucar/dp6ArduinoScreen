#include "voice_service.h"
#include "FS.h"
#include "SD_MMC.h"
#include <freertos/queue.h>

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
            return "VOICE";
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

    SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0); // 1-bit
    // SD_MMC.setFrequency(20000000);          // 20 MHz (más estable) (opcional)

    if (!SD_MMC.begin("/sdcard", true))
    { // true = 1-bit
        Serial.println("SD_MMC.begin() FAIL");
        return;
    }
    // Revisa que tipo de memoria es (opcional)
    uint8_t type = SD_MMC.cardType();
    if (type == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("Card type: ");
    if (type == CARD_MMC)
        Serial.println("MMC");
    else if (type == CARD_SD)
        Serial.println("SDSC");
    else if (type == CARD_SDHC)
        Serial.println("SDHC/SDXC");
    else
        Serial.println("UNKNOWN");

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0-21 típicamente

    _queue = xQueueCreate(5, sizeof(char[64]));

    if (_queue == NULL)
    {
        Serial.println("FATAL: No se pudo crear la cola (Falta de RAM)");
        return;
    }

    BaseType_t stackSize = 8192;

    // Creamos una tarea en el Core 0 (el Core 1 suele ser para la UI/LVGL)
    BaseType_t result = xTaskCreatePinnedToCore(
        this->audioTask,
        "AudioTask",
        stackSize,
        this, // Pasamos la instancia de la clase como parámetro
        3,    // Prioridad alta para que el audio no tartamudee
        &_taskHandle,
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

    if (instance->_queue == NULL)
    {
        Serial.println("AudioTask: Cola no inicializada. Abortando tarea.");
        vTaskDelete(NULL);
        return;
    }

    char nextFile[64];

    for (;;)
    {
        // Revisamos si hay un nuevo archivo para reproducir sin bloquear el loop
        if (xQueueReceive(instance->_queue, &nextFile, 0) == pdPASS)
        {
            Serial.printf("AudioTask: Intentando reproducir %s\n", nextFile);
            if (instance->audio.isRunning()) {
                instance->audio.stopSong();
            }

            if (!instance->_isValidAudioFile(nextFile)) {
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }
            
            vTaskDelay(pdMS_TO_TICKS(50));

            if (!instance->audio.connecttoFS(SD_MMC, nextFile))
            {
                Serial.printf("AudioTask: Error al conectar con %s\n", nextFile);
                continue;
            }

            vTaskDelay(pdMS_TO_TICKS(50));

            Serial.printf("Conectado exitosamente a %s\n", nextFile);

            unsigned long timeout = millis();
            while (instance->audio.getBitRate() == 0 && (millis() - timeout < 200))
            {
                instance->audio.loop();
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            if (instance->audio.getBitRate() == 0 || instance->audio.getSampleRate() == 0)
            {
                instance->audio.stopSong();
                continue;
            }

            Serial.println("--- Info Real del Archivo ---");
            Serial.printf("Sample Rate detectado: %d Hz\n", instance->audio.getSampleRate());
            Serial.printf("Bit Rate detectado: %d bps\n", instance->audio.getBitRate());
            
            while (instance->audio.isRunning())
            {
                Serial.printf("loop: %d\n", millis() - timeout);
                instance->audio.loop();
                
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            
            Serial.printf("Finish Audio: %s\n", nextFile);
            
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        
    }
}

void VoiceService::play(BeepType beepType)
{

    if (!_queue) {
        return;
    }

    char filePath[64];
    
    Serial.printf("BeepType %d\n", beepType);

    snprintf(filePath, sizeof(filePath), "/%s.mp3", getBeepTypeeName(beepType));

    if (xQueueSend(_queue, filePath, portMAX_DELAY) != pdPASS) 
    {
        Serial.println("Error: Cola de audio llena");
    }

}

void VoiceService::play(VoiceType voiceType, VoiceMessage messageType)
{

    if (!_queue) {
        return;
    }

    char filePath[64];

    Serial.printf("VoiceType %d, messageType: %d\n", voiceType, messageType);

    snprintf(filePath, sizeof(filePath), "/%s/%s.mp3", getVoiceTypeName(voiceType), getVoiceMessageName(messageType));

    if (xQueueSend(_queue, filePath, portMAX_DELAY) != pdPASS) 
    {
        Serial.println("Error: Cola de audio llena");
    }
}

bool VoiceService::_isValidAudioFile(const char *filePath)
{
    Serial.printf("Archivo a reproducir: %s.\n", filePath);
    // Verifica que el archivo exista antes de reproducir
    if (!SD_MMC.exists(filePath))
    {
        Serial.printf("MP3 no existe: %s\n", filePath);
        Serial.println("Cambia MP3_PATH por uno que esté en la raíz o carpeta correcta.");
        return false;
    }
    else
    {
        File testFile = SD_MMC.open(filePath);
        size_t size = testFile.size();

        uint8_t header[4];
        testFile.read(header, 4);
        testFile.close();

        if (size == 0)
        {
            Serial.printf("Error: El archivo %s está vacío (0 bytes).\n", filePath);
            return false;
        }
        else
        {
            Serial.printf("Size %d del archivo %s.\n", size, filePath);
        }

        // Acepta tanto MP3 puro como MP3 con ID3
        bool isValidMP3 = false;

        // Verifica si empieza con ID3 tag (49 44 33 = "ID3")
        if (header[0] == 0x49 && header[1] == 0x44 && header[2] == 0x33)
        {
            Serial.printf("MP3 con etiqueta ID3 detectada: %s\n", filePath);
            isValidMP3 = true;
        }
        // Verifica si empieza con frame MP3 (0xFF 0xFx)
        else if (header[0] == 0xFF && (header[1] & 0xE0) == 0xE0)
        {
            Serial.printf("MP3 puro detectado: %s\n", filePath);
            isValidMP3 = true;
        }

        if (!isValidMP3)
        {
            Serial.printf("Error: %s no parece ser un MP3 válido\n", filePath);
            Serial.printf("Header: %02X %02X %02X %02X\n", header[0], header[1], header[2], header[3]);
            return false;
        }

        Serial.printf("Encolando archivo: %s\n", filePath);
        return true;
    
    }
}

void VoiceService::setVolume(uint8_t volume)
{
    audio.setVolume(volume);
}