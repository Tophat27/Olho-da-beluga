#ifndef CONFIG_H
#define CONFIG_H

// Configurações WiFi
#define WIFI_SSID "Com_Virus3_Twibi"
#define WIFI_PASSWORD "0166403808"

// Configurações de detecção
#define MOTION_THRESHOLD 0.05f        // Limiar para detecção de movimento
#define INTERACTION_THRESHOLD 0.1f    // Limiar para detecção de interação
#define FRAME_INTERVAL 100            // Intervalo entre frames (ms)
#define TOP_REGION_RATIO 0.33f        // Proporção da região superior para análise

// Configurações da câmera
#define CAMERA_FRAME_SIZE FRAMESIZE_QVGA
#define CAMERA_JPEG_QUALITY 10
#define CAMERA_FB_COUNT 2

// Pinos da câmera AI-Thinker ESP32-CAM
#define PIN_PWDN 32
#define PIN_RESET -1
#define PIN_XCLK 0
#define PIN_SIOD 26
#define PIN_SIOC 27
#define PIN_D7 35
#define PIN_D6 34
#define PIN_D5 39
#define PIN_D4 36
#define PIN_D3 21
#define PIN_D2 19
#define PIN_D1 18
#define PIN_D0 5
#define PIN_VSYNC 25
#define PIN_HREF 23
#define PIN_PCLK 22

// Configurações específicas para AI-Thinker ESP32-CAM
#define XCLK_FREQ 20000000  // 20MHz para AI-Thinker
#define PIXEL_FORMAT PIXFORMAT_JPEG
#define FRAME_SIZE FRAMESIZE_QVGA
#define JPEG_QUALITY 12
#define FB_COUNT 1

#endif
