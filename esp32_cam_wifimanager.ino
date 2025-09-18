/*
 * ESP32-CAM com WiFiManager
 * Detecção de pose e monitoramento de interação
 * 
 * Funcionalidades:
 * - WiFiManager para configuração automática de rede
 * - Portal de configuração web
 * - Detecção de movimento simplificada
 * - Interface web de controle
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

// Configurações do servidor
WebServer server(80);

// Variáveis de controle
unsigned long lastFrameTime = 0;
unsigned long interactionStartTime = 0;
bool isInteracting = false;
float totalInteractionTime = 0.0;

// Pinos específicos para AI-Thinker ESP32-CAM
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

// Função para inicializar a câmera
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
  
  // Configurações otimizadas
  if(psramFound()){
    Serial.println("PSRAM encontrada - configuração otimizada");
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    Serial.println("PSRAM não encontrada - configuração básica");
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("ERRO ao inicializar câmera: 0x%x\n", err);
    return false;
  }
  
  Serial.println("✓ Câmera inicializada com sucesso!");
  
  // Configurações adicionais
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    Serial.printf("✓ Sensor detectado: PID=0x%x\n", s->id.PID);
    s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_quality(s, config.jpeg_quality);
  }
  
  return true;
}

// Função para detectar movimento
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
  
  for (size_t i = 0; i < fb->len; i += 15) {
    if (abs((int)fb->buf[i] - (int)prevFrame[i]) > 25) {
      diffPixels++;
    }
  }
  
  memcpy(prevFrame, fb->buf, fb->len);
  
  float motionRatio = (float)diffPixels / (totalPixels / 15);
  return motionRatio > 0.03f;
}

// Função para detectar interação
bool detectInteraction(camera_fb_t* fb) {
  if (!fb || !fb->buf) return false;
  
  int width = 320;
  int height = 240;
  int topRegionHeight = height / 3;
  
  int brightPixels = 0;
  int totalPixels = 0;
  
  for (int y = 0; y < topRegionHeight; y += 3) {
    for (int x = 0; x < width; x += 3) {
      int pixelIndex = (y * width + x) * 3;
      if (pixelIndex + 2 < fb->len) {
        uint8_t r = fb->buf[pixelIndex];
        uint8_t g = fb->buf[pixelIndex + 1];
        uint8_t b = fb->buf[pixelIndex + 2];
        
        if (r > 180 && g > 180 && b > 180) {
          brightPixels++;
        }
        totalPixels++;
      }
    }
  }
  
  if (totalPixels == 0) return false;
  
  float brightRatio = (float)brightPixels / totalPixels;
  return brightRatio > 0.08f;
}

// Processar detecção
void processDetection() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || !fb->buf) {
    return;
  }
  
  bool hasMotion = detectMotion(fb);
  bool hasInteraction = detectInteraction(fb);
  
  unsigned long currentTime = millis();
  
  if (hasInteraction) {
    if (!isInteracting) {
      isInteracting = true;
      interactionStartTime = currentTime;
      Serial.println("Interação iniciada");
    } else {
      totalInteractionTime += (currentTime - interactionStartTime) / 1000.0;
      interactionStartTime = currentTime;
    }
  } else {
    if (isInteracting) {
      isInteracting = false;
      Serial.println("Interação finalizada");
    }
  }
  
  esp_camera_fb_return(fb);
}

// Handler para stream de vídeo
void handleRoot() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || !fb->buf) {
    server.send(500, "text/plain", "Erro ao capturar frame");
    return;
  }
  
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(fb->len));
  server.sendHeader("Cache-Control", "no-cache");
  
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
  doc["wifiRSSI"] = WiFi.RSSI();
  
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
  html += "<p><strong>WiFi RSSI:</strong> <span id='rssi'>0</span> dBm</p>";
  html += "<button onclick='resetCounter()'>Resetar Contador</button> ";
  html += "<button onclick='location.reload()'>Atualizar Página</button>";
  html += "<br><br><img src='/' width='320' height='240' style='border: 2px solid #333;' onerror='this.style.display=\"none\"; document.getElementById(\"error\").style.display=\"block\";'>";
  html += "<div id='error' style='display:none; color:red;'>Erro ao carregar imagem da câmera</div>";
  html += "<script>";
  html += "function updateStatus() {";
  html += "  fetch('/status').then(r => r.json()).then(data => {";
  html += "    document.getElementById('status').textContent = data.isInteracting ? 'Interagindo' : 'Inativo';";
  html += "    document.getElementById('time').textContent = data.totalTime.toFixed(1) + 's';";
  html += "    document.getElementById('memory').textContent = data.freeHeap;";
  html += "    document.getElementById('rssi').textContent = data.wifiRSSI;";
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

// Handler para configuração WiFi
void handleConfig() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Configuração WiFi</title></head><body>";
  html += "<h1>Configuração WiFi</h1>";
  html += "<p>Para reconfigurar a rede WiFi, acesse o portal de configuração:</p>";
  html += "<button onclick='startConfig()'>Iniciar Configuração WiFi</button>";
  html += "<br><br><a href='/control'>Voltar ao Controle</a>";
  html += "<script>";
  html += "function startConfig() {";
  html += "  if(confirm('Isso irá reiniciar o ESP32 e abrir o portal de configuração. Continuar?')) {";
  html += "    fetch('/startconfig').then(() => {";
  html += "      alert('Reiniciando... Acesse a rede ESP32-CAM para configurar');";
  html += "    });";
  html += "  }";
  html += "}";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

// Handler para iniciar configuração
void handleStartConfig() {
  server.send(200, "text/plain", "Reiniciando para modo de configuração...");
  delay(1000);
  WiFiManager wm;
  wm.resetSettings();
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== ESP32-CAM com WiFiManager ===");
  
  // Inicializar câmera
  if (!initCamera()) {
    Serial.println("FALHA: Câmera não funcionou. Verifique as conexões.");
    while(1) {
      delay(1000);
      Serial.println("Aguardando...");
    }
  }
  
  // Configurar WiFiManager
  WiFiManager wm;
  wm.setConfigPortalTimeout(180); // 3 minutos para configurar
  wm.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  
  // Tentar conectar ou abrir portal de configuração
  if (!wm.autoConnect("ESP32-CAM-Config")) {
    Serial.println("Falha ao conectar e timeout do portal de configuração");
    delay(3000);
    ESP.restart();
  }
  
  Serial.println("WiFi conectado com sucesso!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  
  // Configurar rotas
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/reset", handleReset);
  server.on("/control", handleControl);
  server.on("/config", handleConfig);
  server.on("/startconfig", handleStartConfig);
  
  server.begin();
  Serial.println("Servidor iniciado!");
  Serial.println("Acesse: http://" + WiFi.localIP().toString() + "/control");
  Serial.println("Configuração: http://" + WiFi.localIP().toString() + "/config");
}

void loop() {
  server.handleClient();
  
  // Processar detecção a cada 100ms
  if (millis() - lastFrameTime >= 100) {
    processDetection();
    lastFrameTime = millis();
  }
  
  // Imprimir status a cada 30 segundos
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 30000) {
    Serial.printf("Status - Interagindo: %s | Tempo: %.1fs | Memória: %d bytes | RSSI: %d dBm\n", 
                  isInteracting ? "Sim" : "Não", totalInteractionTime, ESP.getFreeHeap(), WiFi.RSSI());
    lastPrint = millis();
  }
}
