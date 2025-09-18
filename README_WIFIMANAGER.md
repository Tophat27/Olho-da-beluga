# ESP32-CAM com WiFiManager

Versão otimizada com WiFiManager para configuração automática de rede WiFi.

## Vantagens do WiFiManager

✅ **Configuração Automática** - Não precisa editar código para mudar rede
✅ **Portal Web** - Interface amigável para configurar WiFi
✅ **Fallback** - Se não conectar, abre portal de configuração
✅ **Reset Fácil** - Botão para reconfigurar rede
✅ **Sem Hardcode** - Credenciais não ficam no código

## Instalação

### **1. Instalar Biblioteca WiFiManager**

**Via Arduino IDE:**
1. Abra o Arduino IDE
2. Vá em `Ferramentas` → `Gerenciar Bibliotecas`
3. Procure por "WiFiManager"
4. Instale a biblioteca de **tzapu**

**Via URL:**
```
https://github.com/tzapu/WiFiManager
```

### **2. Configurar Placa**
- **Placa:** "ESP32 Wrover Module"
- **Partition Scheme:** "Huge APP (3MB No OTA/1MB SPIFFS)"
- **PSRAM:** "Enabled"
- **CPU Frequency:** "240MHz"

### **3. Carregar Código**
1. Abra o arquivo `esp32_cam_wifimanager.ino`
2. Compile e carregue no ESP32-CAM
3. Abra o Serial Monitor (115200 baud)

## Como Usar

### **Primeira Configuração:**

1. **Carregue o código** no ESP32-CAM
2. **Aguarde** a inicialização (ver Serial Monitor)
3. **Conecte-se** à rede "ESP32-CAM-Config" (sem senha)
4. **Abra o navegador** - será redirecionado automaticamente
5. **Selecione sua rede WiFi** e digite a senha
6. **Clique em "Salvar"**
7. **Aguarde** a conexão (LED piscará)

### **Uso Normal:**

Após configurado, o ESP32-CAM:
- Conecta automaticamente à rede salva
- Abre servidor web no IP obtido
- Mostra IP no Serial Monitor

### **Reconfigurar Rede:**

1. Acesse `http://IP_DO_ESP32/config`
2. Clique em "Iniciar Configuração WiFi"
3. Siga os passos da primeira configuração

## Funcionalidades

### **Endpoints Disponíveis:**
- `GET /` - Stream de vídeo
- `GET /control` - Interface de controle
- `GET /status` - Status JSON
- `GET /reset` - Resetar contador
- `GET /config` - Configuração WiFi

### **Interface de Controle:**
- Visualização do stream de vídeo
- Status em tempo real
- Informações de memória e WiFi
- Botões de controle

## Exemplo de Uso

### **Serial Monitor:**
```
=== ESP32-CAM com WiFiManager ===
Inicializando câmera AI-Thinker ESP32-CAM...
✓ Câmera inicializada com sucesso!
✓ Sensor detectado: PID=0x...
WiFi conectado com sucesso!
IP: 192.168.1.100
RSSI: -45
Servidor iniciado!
Acesse: http://192.168.1.100/control
```

### **Navegador:**
1. Acesse `http://192.168.1.100/control`
2. Veja o stream de vídeo
3. Monitore status em tempo real
4. Use botões de controle

## Solução de Problemas

### **Portal não abre:**
- Verifique se está conectado à rede "ESP32-CAM-Config"
- Aguarde alguns segundos após conectar
- Tente acessar `192.168.4.1` diretamente

### **Não conecta após configurar:**
- Verifique se a senha está correta
- Confirme que a rede é 2.4GHz
- Verifique força do sinal
- Use o botão de reconfiguração

### **Câmera não funciona:**
- Verifique conexões dos pinos
- Confirme alimentação 5V/2A
- Teste com código simples primeiro

### **Reset completo:**
```cpp
// Adicione este código no setup() para resetar tudo
WiFiManager wm;
wm.resetSettings();
ESP.restart();
```

## Configurações Avançadas

### **Personalizar Portal:**
```cpp
WiFiManager wm;
wm.setConfigPortalTimeout(300); // 5 minutos
wm.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
wm.setAPCallback([](WiFiManager *myWiFiManager) {
  Serial.println("Portal de configuração iniciado");
});
```

### **Adicionar Parâmetros Customizados:**
```cpp
WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", "192.168.1.1", 40);
wm.addParameter(&custom_mqtt_server);
```

## Comparação com Versão Anterior

| Aspecto | Versão Anterior | Com WiFiManager |
|---------|----------------|-----------------|
| **Configuração** | Editar código | Interface web |
| **Mudança de Rede** | Recompilar | Portal web |
| **Facilidade** | Difícil | Fácil |
| **Flexibilidade** | Baixa | Alta |
| **Manutenção** | Complexa | Simples |

## Próximos Passos

1. **Teste básico** - Verifique se a câmera funciona
2. **Configure WiFi** - Use o portal de configuração
3. **Teste interface** - Acesse a página de controle
4. **Monitore** - Verifique logs no Serial Monitor
5. **Personalize** - Ajuste configurações conforme necessário

## Dicas Importantes

- **Primeira vez:** Sempre teste com código simples primeiro
- **Alimentação:** Use fonte 5V/2A (não USB)
- **Rede:** Funciona apenas com 2.4GHz
- **Reset:** Mantenha botão RESET pressionado por 10s para reset completo
- **Logs:** Sempre monitore o Serial Monitor para debug
