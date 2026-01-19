# VS CODE configuration

```json
{
    "configurations": [
        {
            "name": "Mac",
            "includePath": [
                "${workspaceFolder}",
                "${workspaceFolder}/**",
                "/Users/eric/Documents/Arduino/libraries/lvgl/src",
                "~/Library/Arduino15/packages/esp32/hardware/esp32/*/cores/esp32",
                "~/Library/Arduino15/packages/esp32/hardware/esp32/*/libraries/**",
                "/Users/eric/Library/Arduino15/packages/esp32/tools/esp32-arduino-libs/*/esp32s3/include/*/include",
                "/Users/eric/Library/Arduino15/packages/esp32/tools/esp32-arduino-libs/*/esp32s3/include/*/*/include",
                "/Users/eric/Downloads/JC3248W535EN-main/JC3248W535EN/1-Demo/Demo_Arduino/esp32s3/opi_opi/include",
                "~/Documents/Arduino/libraries/**"
            ],
            "defines": [
                "ARDUINO=10819",
                "ESP32",
                "ARDUINO_ESP32S3_DEV",
                "FREERTOS"
            ],
            "compilerPath": "/Users/eric/Library/Arduino15/packages/esp32/tools/esp-x32/2507/bin/xtensa-esp32s3-elf-gcc",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "gcc-arm"
        }
    ],
    "version": 4
}
```