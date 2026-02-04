#include "display_service.h"

#include <ctype.h> // Necesaria para isdigit()

namespace
{
    const char *extractDigits(const char *input)
    {
        static char output[7];
        int totalDigits = 0;
        const char *temp = input;

        // 1. Contar cuántos dígitos hay en total (máximo 6)
        while (*temp != '\0' && totalDigits < 6)
        {
            if (isdigit(*temp))
            {
                totalDigits++;
            }
            temp++;
        }

        // 2. Inicializar con espacios para limpiar el display
        memset(output, ' ', 6);
        output[6] = '\0';

        // 3. Llenar de derecha a izquierda
        int writeIndex = 5; // Última posición del array (índice 5 para 6 dígitos)

        // Retrocedemos en el input para capturar los últimos dígitos encontrados
        // (o empezamos desde el final de la cadena original)
        int inputLen = strlen(input);
        for (int i = inputLen - 1; i >= 0 && writeIndex >= (6 - totalDigits); i--)
        {
            if (isdigit(input[i]))
            {
                output[writeIndex] = input[i];
                writeIndex--;
            }
        }

        return output;
    }
}

void DisplayService::init()
{
    display.begin();
    display.setBrightness(BRIGHT_HIGH);
    display.clear();
    // display.showString("123456", 6U, 0U, 0b00010000);
    showString("S/ 0.00");
}

void DisplayService::showString(const char *str)
{
    auto digits = extractDigits(str);
    display.clear();
    display.showString(digits, 6U, 0U, 0b00010000);
}