/*
 * Teste simples da câmera AI-Thinker ESP32-CAM
 * Este código apenas testa se a câmera está funcionando
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// Configurações WiFi
const char* ssid = "Com_Virus3_Twibi";
const char* password = "0166403808";

WebServer server(80);

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

bool initCamera() {
  Serial.println("=== TESTE DA CÂMERA AI-THINKER ESP32-CAM ===");
  
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
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  Serial.println("Inicializando câmera...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("ERRO: Falha ao inicializar câmera (0x%x)\n", err);
    Serial.println("Possíveis causas:");
    Serial.println("1. Conexões dos pinos incorretas");
    Serial.println("2. Câmera danificada");
    Serial.println("3. Alimentação insuficiente");
    return false;
  }
  
  Serial.println("✓ Câmera inicializada com sucesso!");
  
  // Verificar sensor
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    Serial.printf("✓ Sensor detectado: PID=0x%x\n", s->id.PID);
  } else {
    Serial.println("⚠ Aviso: Sensor não detectado");
  }
  
  // Teste de captura
  Serial.println("Testando captura...");
  camera_fb_t* fb = esp_camera_fb_get();
  if (fb && fb->buf) {
    Serial.printf("✓ Captura OK - %dx%d, %d bytes\n", fb->width, fb->height, fb->len);
    esp_camera_fb_return(fb);
  } else {
    Serial.println("✗ ERRO: Falha na captura");
    return false;
  }
  
  return true;
}

void handleRoot() {
  Serial.println("Requisição de imagem recebida");
  
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || !fb->buf) {
    Serial.println("ERRO: Falha ao capturar frame");
    server.send(500, "text/plain", "Erro ao capturar frame");
    return;
  }
  
  Serial.printf("Enviando imagem: %dx%d, %d bytes\n", fb->width, fb->height, fb->len);
  
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(fb->len));
  server.sendHeader("Cache-Control", "no-cache");
  
  server.send(200, "image/jpeg", "");
  server.sendContent((char*)fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
  Serial.println("Imagem enviada com sucesso");
}

void handleTest() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Teste Câmera</title></head><body>";
  html += "<h1>Teste da Câmera ESP32-CAM</h1>";
  html += "<p>Se você conseguir ver a imagem abaixo, a câmera está funcionando!</p>";
  html += "<img src='/' width='320' height='240' style='border: 2px solid #333;'>";
  html += "<p><a href='/'>Atualizar imagem</a></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== TESTE DA CÂMERA AI-THINKER ESP32-CAM ===");
  
  // Inicializar câmera
  if (!initCamera()) {
    Serial.println("FALHA: Câmera não funcionou. Verifique as conexões.");
    while(1) {
      delay(1000);
      Serial.println("Aguardando...");
    }
  }
  
  // Conectar WiFi
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi conectado! IP: ");
  Serial.println(WiFi.localIP());
  
  // Configurar servidor
  server.on("/", handleRoot);
  server.on("/test", handleTest);
  server.begin();
  
  Serial.println("Servidor iniciado!");
  Serial.println("Acesse: http://" + WiFi.localIP().toString() + "/test");
  Serial.println("Para ver a imagem: http://" + WiFi.localIP().toString() + "/");
}

void loop() {
  server.handleClient();
  delay(10);
}
