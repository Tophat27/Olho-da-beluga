# AI-Thinker ESP32-CAM - Detecção de Pose

Código específico para a placa **AI-Thinker ESP32-CAM** com câmera OV2640.

## Características da AI-Thinker ESP32-CAM

### **Hardware:**
- **Microcontrolador:** ESP32 com WiFi e Bluetooth
- **Câmera:** OV2640 (2MP)
- **Memória:** 4MB Flash + PSRAM opcional
- **Pinos:** Configuração específica para AI-Thinker

### **Configuração de Pinos:**
```
PWDN_GPIO_NUM     32
RESET_GPIO_NUM    -1
XCLK_GPIO_NUM      0
SIOD_GPIO_NUM     26
SIOC_GPIO_NUM     27
Y9_GPIO_NUM       35
Y8_GPIO_NUM       34
Y7_GPIO_NUM       39
Y6_GPIO_NUM       36
Y5_GPIO_NUM       21
Y4_GPIO_NUM       19
Y3_GPIO_NUM       18
Y2_GPIO_NUM        5
VSYNC_GPIO_NUM    25
HREF_GPIO_NUM     23
PCLK_GPIO_NUM     22
```

## Instalação

### **1. Preparar o Ambiente**
1. Instale o Arduino IDE
2. Adicione a URL do ESP32: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Instale as bibliotecas:
   - ArduinoJson (versão 6.x)
   - ESP32 Camera (já incluída)

### **2. Configurar WiFi**
Edite as credenciais no arquivo `ai_thinker_esp32_cam.ino`:
```cpp
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
```

### **3. Configurações do Arduino IDE**
- **Placa:** "ESP32 Wrover Module"
- **Partition Scheme:** "Huge APP (3MB No OTA/1MB SPIFFS)"
- **PSRAM:** "Enabled" (se disponível)
- **CPU Frequency:** "240MHz (WiFi/BT)"
- **Flash Mode:** "QIO"
- **Flash Size:** "4MB (32Mb)"
- **Flash Frequency:** "80MHz"

### **4. Compilar e Carregar**
1. Conecte o ESP32-CAM ao computador via USB
2. Mantenha o botão BOOT pressionado
3. Pressione o botão RESET
4. Solte o botão RESET
5. Solte o botão BOOT
6. Compile e carregue o código

## Funcionalidades

### **Endpoints Disponíveis:**
- `GET /` - Stream de vídeo JPEG
- `GET /status` - Status JSON com dados de interação
- `GET /reset` - Resetar contador de tempo
- `GET /control` - Página web de controle

### **Exemplo de Resposta da API:**
```json
{
  "isInteracting": true,
  "totalTime": 15.3,
  "uptime": 120,
  "freeHeap": 123456
}
```

## Configurações Otimizadas

### **Para AI-Thinker ESP32-CAM:**
- **Resolução:** QVGA (320x240) - otimizada para performance
- **Qualidade JPEG:** 10-12 (balance entre qualidade e velocidade)
- **FPS:** ~10 FPS (limitado pelo processamento)
- **Memória:** 1-2 buffers dependendo da PSRAM

### **Detecção de Movimento:**
- **Sensibilidade:** 3% de pixels diferentes
- **Amostragem:** 1 em cada 15 pixels
- **Threshold:** 25 níveis de diferença

### **Detecção de Interação:**
- **Região:** Parte superior (1/3 da imagem)
- **Sensibilidade:** 8% de pixels claros
- **Amostragem:** 1 em cada 3 pixels

## Solução de Problemas

### **Câmera não inicializa:**
- Verifique as conexões dos pinos
- Confirme a alimentação 5V/2A
- Teste com exemplo básico da biblioteca ESP32 Camera

### **WiFi não conecta:**
- Verifique SSID e senha
- Confirme que a rede é 2.4GHz
- Verifique a força do sinal
- Reinicie o ESP32

### **Performance baixa:**
- Diminua a qualidade JPEG
- Aumente o intervalo de detecção
- Verifique alimentação adequada
- Monitore memória livre

### **Erro de compilação:**
- Verifique se as bibliotecas estão instaladas
- Confirme a versão do Arduino IDE
- Verifique as configurações da placa

## Monitoramento

### **Via Serial Monitor:**
```
Iniciando AI-Thinker ESP32-CAM Pose Detection
Câmera inicializada com sucesso
WiFi conectado! IP: 192.168.1.100
Servidor iniciado
Acesse: http://192.168.1.100/control
Status - Interagindo: Sim | Tempo: 15.3s | Memória: 123456 bytes
```

### **Via Web Interface:**
Acesse `http://IP_DO_ESP32/control` para:
- Visualizar o stream de vídeo
- Monitorar status em tempo real
- Resetar contador
- Ver informações de memória

## Diferenças da Versão Padrão

| Aspecto | Versão Padrão | AI-Thinker |
|---------|---------------|------------|
| **Pinos** | Genéricos | Específicos AI-Thinker |
| **Performance** | Básica | Otimizada |
| **Interface** | Simples | Web completa |
| **Monitoramento** | Serial | Web + Serial |
| **Configuração** | Manual | Automática |

## Limitações

- **Resolução:** Limitada a QVGA para performance
- **FPS:** ~10 FPS máximo
- **Precisão:** Detecção simplificada (não YOLO real)
- **Memória:** 4MB Flash + PSRAM limitada
- **Processamento:** CPU única (sem GPU)

## Próximos Passos

1. **Teste básico:** Verifique se a câmera funciona
2. **Configuração WiFi:** Conecte à sua rede
3. **Teste de detecção:** Mova os braços na frente da câmera
4. **Monitoramento:** Use a interface web
5. **Ajustes:** Modifique sensibilidade conforme necessário
