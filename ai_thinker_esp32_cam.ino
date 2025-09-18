/*
 * Código específico para AI-Thinker ESP32-CAM
 * Detecção de pose e monitoramento de interação
 * 
 * Configurações otimizadas para:
 * - AI-Thinker ESP32-CAM
 * - Câmera OV2640
 * - WiFi 2.4GHz
 * - Detecção de movimento simplificada
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Configurações WiFi
const char* ssid = "Com_Virus3_Twibi";
const char* password = "0166403808";

// Configurações do servidor
WebServer server(80);

// Variáveis de controle
unsigned long lastFrameTime = 0;
unsigned long interactionStartTime = 0;
bool isInteracting = false;
float totalInteractionTime = 0.0;

// Configurações específicas para AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Função para inicializar a câmera AI-Thinker
bool initCamera() {
  Serial.println("Inicializando câmera AI-Thinker ESP32-CAM...");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Configurações otimizadas para AI-Thinker
  if(psramFound()){
    Serial.println("PSRAM encontrada - usando configuração otimizada");
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    Serial.println("PSRAM não encontrada - usando configuração básica");
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("ERRO ao inicializar câmera: 0x%x\n", err);
    Serial.println("Verifique as conexões dos pinos da câmera");
    return false;
  }
  
  Serial.println("Câmera inicializada com sucesso!");
  
  // Configurações adicionais da câmera
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    Serial.printf("Sensor detectado: PID=0x%x\n", s->id.PID);
    if (s->id.PID == OV3660_PID) {
      s->set_vflip(s, 1);
      s->set_brightness(s, 1);
      s->set_saturation(s, -2);
    }
    s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_quality(s, config.jpeg_quality);
  } else {
    Serial.println("AVISO: Sensor não detectado");
  }
  
  // Teste de captura
  camera_fb_t* test_fb = esp_camera_fb_get();
  if (test_fb) {
    Serial.printf("Teste de captura OK - Tamanho: %dx%d, Buffer: %d bytes\n", 
                  test_fb->width, test_fb->height, test_fb->len);
    esp_camera_fb_return(test_fb);
  } else {
    Serial.println("ERRO: Falha no teste de captura");
    return false;
  }
  
  return true;
}

// Função para detectar movimento simples
bool detectMotion(camera_fb_t* fb) {
  if (!fb || !fb->buf) return false;
  
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
  
  for (size_t i = 0; i < fb->len; i += 15) { // Amostragem para performance
    if (abs((int)fb->buf[i] - (int)prevFrame[i]) > 25) {
      diffPixels++;
    }
  }
  
  // Atualizar frame anterior
  memcpy(prevFrame, fb->buf, fb->len);
  
  // Detectar movimento significativo
  float motionRatio = (float)diffPixels / (totalPixels / 15);
  return motionRatio > 0.03f; // 3% de pixels diferentes
}

// Função para detectar interação na região superior
bool detectInteraction(camera_fb_t* fb) {
  if (!fb || !fb->buf) return false;
  
  // Análise da parte superior da imagem (região dos braços)
  int width = 320;  // QVGA width
  int height = 240; // QVGA height
  int topRegionHeight = height / 3; // Região superior
  
  // Contar pixels com alta intensidade na região superior
  int brightPixels = 0;
  int totalPixels = 0;
  
  // Amostragem da região superior
  for (int y = 0; y < topRegionHeight; y += 3) {
    for (int x = 0; x < width; x += 3) {
      int pixelIndex = (y * width + x) * 3; // JPEG é RGB
      if (pixelIndex + 2 < fb->len) {
        uint8_t r = fb->buf[pixelIndex];
        uint8_t g = fb->buf[pixelIndex + 1];
        uint8_t b = fb->buf[pixelIndex + 2];
        
        // Detectar pixels claros (possível movimento de braços)
        if (r > 180 && g > 180 && b > 180) {
          brightPixels++;
        }
        totalPixels++;
      }
    }
  }
  
  if (totalPixels == 0) return false;
  
  float brightRatio = (float)brightPixels / totalPixels;
  return brightRatio > 0.08f; // 8% de pixels claros na região superior
}

// Função para processar detecção
void processDetection() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || !fb->buf) {
    return;
  }
  
  // Detectar movimento/interação
  bool hasMotion = detectMotion(fb);
  bool hasInteraction = detectInteraction(fb);
  
  unsigned long currentTime = millis();
  
  // Lógica de tempo de interação
  if (hasInteraction) {
    if (!isInteracting) {
      isInteracting = true;
      interactionStartTime = currentTime;
      Serial.println("Interação iniciada");
    } else {
      // Atualizar tempo total de interação
      totalInteractionTime += (currentTime - interactionStartTime) / 1000.0;
      interactionStartTime = currentTime;
    }
  } else {
    if (isInteracting) {
      isInteracting = false;
      Serial.println("Interação finalizada");
    }
  }
  
  // Liberar buffer
  esp_camera_fb_return(fb);
}

// Handler para stream de vídeo
void handleRoot() {
  static unsigned long lastCapture = 0;
  static int captureCount = 0;
  
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || !fb->buf) {
    Serial.println("ERRO: Falha ao capturar frame");
    server.send(500, "text/plain", "Erro ao capturar frame");
    return;
  }
  
  // Log de debug a cada 10 capturas
  captureCount++;
  if (captureCount % 10 == 0) {
    Serial.printf("Captura #%d - Tamanho: %dx%d, Buffer: %d bytes\n", 
                  captureCount, fb->width, fb->height, fb->len);
  }
  
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(fb->len));
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  
  // Usar sendContent para melhor compatibilidade
  server.send(200, "image/jpeg", "");
  server.sendContent((char*)fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
}

// Handler para status JSON
void handleStatus() {
  StaticJsonDocument<200> doc;
  doc["isInteracting"] = isInteracting;
  doc["totalTime"] = totalInteractionTime;
  doc["uptime"] = millis() / 1000;
  doc["freeHeap"] = ESP.getFreeHeap();
  
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

// Handler para página de controle
void handleControl() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32-CAM Control</title></head><body>";
  html += "<h1>ESP32-CAM Pose Detection</h1>";
  html += "<p><strong>Status:</strong> <span id='status'>Carregando...</span></p>";
  html += "<p><strong>Tempo de Interação:</strong> <span id='time'>0.0s</span></p>";
  html += "<p><strong>Memória Livre:</strong> <span id='memory'>0</span> bytes</p>";
  html += "<button onclick='resetCounter()'>Resetar Contador</button>";
  html += "<br><br><img src='/' width='320' height='240' style='border: 2px solid #333;' onerror='this.style.display=\"none\"; document.getElementById(\"error\").style.display=\"block\";'>";
  html += "<div id='error' style='display:none; color:red;'>Erro ao carregar imagem da câmera</div>";
  html += "<script>";
  html += "function updateStatus() {";
  html += "  fetch('/status').then(r => r.json()).then(data => {";
  html += "    document.getElementById('status').textContent = data.isInteracting ? 'Interagindo' : 'Inativo';";
  html += "    document.getElementById('time').textContent = data.totalTime.toFixed(1) + 's';";
  html += "    document.getElementById('memory').textContent = data.freeHeap;";
  html += "  }).catch(e => console.log('Erro ao buscar status:', e));";
  html += "}";
  html += "function resetCounter() {";
  html += "  fetch('/reset').then(() => updateStatus()).catch(e => console.log('Erro ao resetar:', e));";
  html += "}";
  html += "setInterval(updateStatus, 1000);";
  html += "updateStatus();";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando AI-Thinker ESP32-CAM Pose Detection");
  
  // Inicializar câmera
  if (!initCamera()) {
    Serial.println("Falha ao inicializar câmera");
    return;
  }
  Serial.println("Câmera inicializada com sucesso");
  
  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi conectado! IP: ");
  Serial.println(WiFi.localIP());
  
  // Configurar rotas
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/reset", handleReset);
  server.on("/control", handleControl);
  
  server.begin();
  Serial.println("Servidor iniciado");
  Serial.println("Acesse: http://" + WiFi.localIP().toString() + "/control");
}

void loop() {
  server.handleClient();
  
  // Processar detecção a cada 100ms
  if (millis() - lastFrameTime >= 100) {
    processDetection();
    lastFrameTime = millis();
  }
  
  // Imprimir status a cada 10 segundos
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 10000) {
    Serial.printf("Status - Interagindo: %s | Tempo: %.1fs | Memória: %d bytes\n", 
                  isInteracting ? "Sim" : "Não", totalInteractionTime, ESP.getFreeHeap());
    lastPrint = millis();
  }
}
