# ESP32-CAM Pose Detection

Este código implementa detecção de pose e monitoramento de interação para ESP32-CAM em C++.

## Funcionalidades

- ✅ Detecção de movimento baseada em diferenças de pixel
- ✅ Detecção de interação na região superior da imagem
- ✅ Cálculo de tempo total de interação
- ✅ Servidor web para visualização do stream
- ✅ API REST para status e controle
- ✅ Conformidade com regras (sem listas/tuplas)

## Hardware Necessário

- ESP32-CAM
- Fonte de alimentação 5V/2A
- Cabo USB para programação
- Antena WiFi (opcional, mas recomendada)

## Instalação

### 1. Preparar o Ambiente

1. Instale o Arduino IDE
2. Adicione a URL do ESP32: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Instale as bibliotecas:
   - ArduinoJson (versão 6.x)
   - ESP32 Camera (já incluída)

### 2. Configurar WiFi

Edite o arquivo `config.h`:
```cpp
#define WIFI_SSID "SEU_WIFI"
#define WIFI_PASSWORD "SUA_SENHA"
```

### 3. Compilar e Carregar

1. Selecione a placa: "ESP32 Wrover Module"
2. Configure as opções:
   - Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
   - PSRAM: "Enabled"
3. Compile e carregue o código

## Uso

### Acessar o Stream

Após conectar, acesse no navegador:
```
http://IP_DO_ESP32/
```

### API Endpoints

- `GET /` - Stream de vídeo
- `GET /status` - Status JSON com dados de interação
- `GET /reset` - Resetar contador de tempo

### Exemplo de Resposta da API

```json
{
  "isInteracting": true,
  "totalTime": 15.3,
  "motionDetected": true
}
```

## Configurações Avançadas

### Ajustar Sensibilidade

No arquivo `config.h`:
```cpp
#define MOTION_THRESHOLD 0.05f        // Menor = mais sensível
#define INTERACTION_THRESHOLD 0.1f    // Menor = mais sensível
```

### Ajustar Performance

```cpp
#define FRAME_INTERVAL 100            // Menor = mais FPS (mais CPU)
#define CAMERA_JPEG_QUALITY 10        // Menor = melhor qualidade (mais memória)
```

## Limitações

- **Detecção Simplificada**: Não usa YOLO real, apenas análise de movimento
- **Memória Limitada**: Processamento básico para economizar RAM
- **Precisão**: Menos precisa que soluções com IA completa

## Solução de Problemas

### Câmera não inicializa
- Verifique as conexões dos pinos
- Confirme a alimentação 5V/2A
- Teste com exemplo básico da biblioteca

### WiFi não conecta
- Verifique SSID e senha
- Confirme que a rede é 2.4GHz
- Verifique a força do sinal

### Performance baixa
- Aumente `FRAME_INTERVAL`
- Diminua `CAMERA_JPEG_QUALITY`
- Verifique alimentação adequada

## Estrutura do Código

- `esp32_pose_detection.cpp` - Código principal
- `config.h` - Configurações
- `README_ESP32.md` - Esta documentação

## Diferenças do Python Original

| Aspecto | Python Original | C++ ESP32 |
|---------|----------------|-----------|
| YOLO | YOLOv8 pose | Detecção de movimento |
| Processamento | IA completa | Análise de pixels |
| Precisão | Alta | Média |
| Recursos | Computador | ESP32-CAM |
| Memória | Ilimitada | 4MB |
