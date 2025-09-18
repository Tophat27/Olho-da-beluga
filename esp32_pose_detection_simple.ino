#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "config.h"

// Configurações do servidor
WebServer server(80);

// Variáveis de controle
unsigned long lastFrameTime = 0;
unsigned long interactionStartTime = 0;
bool isInteracting = false;
float totalInteractionTime = 0.0;

// Estrutura para keypoints (substituindo arrays)
struct Keypoint {
  float x;
  float y;
  float confidence;
  bool valid;
};

// Estrutura para pessoa detectada
struct Person {
  Keypoint keypoints[17]; // 17 keypoints do COCO pose
  bool hasValidPose;
  float overallConfidence;
};

// Estrutura para configuração da câmera
struct CameraConfig {
  ledc_channel_t ledc_channel;
  ledc_timer_t ledc_timer;
  int pin_d0;
  int pin_d1;
  int pin_d2;
  int pin_d3;
  int pin_d4;
  int pin_d5;
  int pin_d6;
  int pin_d7;
  int pin_xclk;
  int pin_pclk;
  int pin_vsync;
  int pin_href;
  int pin_sscb_sda;
  int pin_sscb_scl;
  int pin_pwdn;
  int pin_reset;
  int xclk_freq_hz;
  int pixel_format;
  int frame_size;
  int jpeg_quality;
  int fb_count;
};

// Configuração da câmera
CameraConfig camConfig = {
  .ledc_channel = LEDC_CHANNEL_0,
  .ledc_timer = LEDC_TIMER_0,
  .pin_d0 = PIN_D0,
  .pin_d1 = PIN_D1,
  .pin_d2 = PIN_D2,
  .pin_d3 = PIN_D3,
  .pin_d4 = PIN_D4,
  .pin_d5 = PIN_D5,
  .pin_d6 = PIN_D6,
  .pin_d7 = PIN_D7,
  .pin_xclk = PIN_XCLK,
  .pin_pclk = PIN_PCLK,
  .pin_vsync = PIN_VSYNC,
  .pin_href = PIN_HREF,
  .pin_sscb_sda = PIN_SIOD,
  .pin_sscb_scl = PIN_SIOC,
  .pin_pwdn = PIN_PWDN,
  .pin_reset = PIN_RESET,
  .xclk_freq_hz = XCLK_FREQ,
  .pixel_format = PIXEL_FORMAT,
  .frame_size = FRAME_SIZE,
  .jpeg_quality = JPEG_QUALITY,
  .fb_count = FB_COUNT
};

// Função para inicializar a câmera
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = camConfig.ledc_channel;
  config.ledc_timer = camConfig.ledc_timer;
  config.pin_d0 = camConfig.pin_d0;
  config.pin_d1 = camConfig.pin_d1;
  config.pin_d2 = camConfig.pin_d2;
  config.pin_d3 = camConfig.pin_d3;
  config.pin_d4 = camConfig.pin_d4;
  config.pin_d5 = camConfig.pin_d5;
  config.pin_d6 = camConfig.pin_d6;
  config.pin_d7 = camConfig.pin_d7;
  config.pin_xclk = camConfig.pin_xclk;
  config.pin_pclk = camConfig.pin_pclk;
  config.pin_vsync = camConfig.pin_vsync;
  config.pin_href = camConfig.pin_href;
  config.pin_sscb_sda = camConfig.pin_sscb_sda;
  config.pin_sscb_scl = camConfig.pin_sscb_scl;
  config.pin_pwdn = camConfig.pin_pwdn;
  config.pin_reset = camConfig.pin_reset;
  config.xclk_freq_hz = camConfig.xclk_freq_hz;
  config.pixel_format = (pixformat_t)camConfig.pixel_format;
  config.frame_size = (framesize_t)camConfig.frame_size;
  config.jpeg_quality = camConfig.jpeg_quality;
  config.fb_count = camConfig.fb_count;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Erro ao inicializar câmera: 0x%x\n", err);
    return false;
  }
  return true;
}

// Função para detectar movimento simples baseada em diferenças de pixel
bool detectSimplePose(camera_fb_t* fb) {
  if (!fb || !fb->buf) return false;
  
  // Análise simples baseada em diferenças de pixel
  // Esta é uma implementação simplificada para ESP32
  static uint8_t* prevFrame = nullptr;
  static size_t prevFrameSize = 0;
  
  if (prevFrame == nullptr) {
    prevFrame = (uint8_t*)malloc(fb->len);
    if (prevFrame) {
      memcpy(prevFrame, fb->buf, fb->len);
      prevFrameSize = fb->len;
    }
    return false;
  }
  
  if (prevFrameSize != fb->len) {
    free(prevFrame);
    prevFrame = (uint8_t*)malloc(fb->len);
    if (prevFrame) {
      memcpy(prevFrame, fb->buf, fb->len);
      prevFrameSize = fb->len;
    }
    return false;
  }
  
  // Calcular diferença entre frames
  int diffPixels = 0;
  int totalPixels = fb->len;
  
  for (size_t i = 0; i < fb->len; i += 10) { // Amostragem para performance
    if (abs((int)fb->buf[i] - (int)prevFrame[i]) > 30) {
      diffPixels++;
    }
  }
  
  // Atualizar frame anterior
  memcpy(prevFrame, fb->buf, fb->len);
  
  // Detectar movimento significativo
  float motionRatio = (float)diffPixels / (totalPixels / 10);
  return motionRatio > MOTION_THRESHOLD;
}

// Função para detectar interação baseada em movimento na parte superior
bool detectInteraction(camera_fb_t* fb) {
  if (!fb || !fb->buf) return false;
  
  // Análise da parte superior da imagem (região dos braços)
  int width = 320;  // QVGA width
  int height = 240; // QVGA height
  int topRegionHeight = height * TOP_REGION_RATIO; // Região superior
  
  // Contar pixels com alta intensidade na região superior
  int brightPixels = 0;
  int totalPixels = 0;
  
  // Amostragem da região superior
  for (int y = 0; y < topRegionHeight; y += 2) {
    for (int x = 0; x < width; x += 2) {
      int pixelIndex = (y * width + x) * 3; // JPEG é RGB
      if (pixelIndex + 2 < fb->len) {
        uint8_t r = fb->buf[pixelIndex];
        uint8_t g = fb->buf[pixelIndex + 1];
        uint8_t b = fb->buf[pixelIndex + 2];
        
        // Detectar pixels claros (possível movimento de braços)
        if (r > 200 && g > 200 && b > 200) {
          brightPixels++;
        }
        totalPixels++;
      }
    }
  }
  
  if (totalPixels == 0) return false;
  
  float brightRatio = (float)brightPixels / totalPixels;
  return brightRatio > INTERACTION_THRESHOLD;
}

// Função para processar detecção de pose
void processPoseDetection() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || !fb->buf) {
    return;
  }
  
  // Detectar movimento/interação
  bool hasMotion = detectSimplePose(fb);
  bool hasInteraction = detectInteraction(fb);
  
  unsigned long currentTime = millis();
  
  // Lógica de tempo de interação
  if (hasInteraction) {
    if (!isInteracting) {
      isInteracting = true;
      interactionStartTime = currentTime;
    } else {
      // Atualizar tempo total de interação
      totalInteractionTime += (currentTime - interactionStartTime) / 1000.0;
      interactionStartTime = currentTime;
    }
  } else {
    if (isInteracting) {
      isInteracting = false;
    }
  }
  
  // Liberar buffer
  esp_camera_fb_return(fb);
}

// Handler para página principal
void handleRoot() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || !fb->buf) {
    server.send(500, "text/plain", "Erro ao capturar frame");
    return;
  }
  
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(fb->len));
  server.send(200, "image/jpeg", String((char*)fb->buf, fb->len));
  esp_camera_fb_return(fb);
}

// Handler para dados de status
void handleStatus() {
  StaticJsonDocument<200> doc;
  doc["isInteracting"] = isInteracting;
  doc["totalTime"] = totalInteractionTime;
  doc["motionDetected"] = detectSimplePose(esp_camera_fb_get());
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

// Handler para resetar contador
void handleReset() {
  totalInteractionTime = 0.0;
  isInteracting = false;
  server.send(200, "text/plain", "Contador resetado");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando ESP32-CAM Pose Detection");
  
  // Inicializar câmera
  if (!initCamera()) {
    Serial.println("Falha ao inicializar câmera");
    return;
  }
  
  // Conectar WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Configurar rotas
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/reset", handleReset);
  
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();
  
  // Processar detecção a cada FRAME_INTERVAL ms
  if (millis() - lastFrameTime >= FRAME_INTERVAL) {
    processPoseDetection();
    lastFrameTime = millis();
  }
  
  // Imprimir status a cada 5 segundos
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 5000) {
    Serial.printf("Interagindo: %s | Tempo: %.1fs\n", 
                  isInteracting ? "Sim" : "Não", totalInteractionTime);
    lastPrint = millis();
  }
}
