# 🌊 Detector de Alagamento - IoT com ESP32

<div align="center">
  <img src="https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white" alt="Arduino">
  <img src="https://img.shields.io/badge/ESP32-000000?style=for-the-badge&logo=espressif&logoColor=white" alt="ESP32">
  <img src="https://img.shields.io/badge/IoT-Internet%20of%20Things-blue?style=for-the-badge" alt="IoT">
  <img src="https://img.shields.io/badge/Telegram-Bot-26A5E4?style=for-the-badge&logo=telegram&logoColor=white" alt="Telegram">
</div>

## 📋 Sobre o Projeto

O **Detector de Alagamento** é um sistema IoT inteligente desenvolvido com **ESP32** para monitoramento automático de níveis de água e detecção de alagamentos. O sistema oferece alertas visuais, sonoros e notificações via **Telegram** para grupos, com interface web para monitoramento em tempo real.

### 🎯 Objetivos do Sistema

- **Monitoramento contínuo** de níveis de água com sensor ultrassônico JSN-SR04T
- **Detecção de chuva** com sensor YL-83 para alertas preventivos
- **Sistema de alertas visuais** com 5 LEDs indicadores coloridos
- **Notificações automáticas** via bot do Telegram para grupos
- **Interface web responsiva** para monitoramento remoto
- **Histórico de eventos** com timestamps e logs detalhados

### 🌟 Características Principais

#### 🚨 **Sistema de Alertas Inteligente**
- **3 Níveis de Status**: Normal (✅), Alerta (⚠️), Perigo (🚨)
- **Histerese configurável** (5cm) para evitar oscilações
- **Limiares personalizáveis**: 50cm (alerta) e 30cm (perigo)
- **Alertas sonoros diferenciados**: intermitente (alerta) e contínuo (perigo)

#### 💡 **Indicadores Visuais (LEDs)**
- 🔵 **LED Azul**: Status da conexão com Internet
- ⚪ **LED Branco**: Status da conexão com Telegram  
- 🔴 **LED Vermelho**: Nível de perigo (pisca rápido)
- 🟡 **LED Amarelo**: Nível de alerta (pisca lento)
- 🟢 **LED Verde**: Status normal (aceso fixo)

#### 📱 **Integração com Telegram**
- **Bot dedicado** para envio de notificações automáticas
- **Suporte a grupos** para alertas em equipe
- **Cooldown inteligente** (5 minutos) para evitar spam
- **Mensagens formatadas** com emojis e informações detalhadas

#### 🌐 **Interface Web Moderna**
- **Dashboard responsivo** com design moderno
- **Atualização automática** via AJAX (3 segundos)
- **Histórico visual** de eventos com cores diferenciadas
- **Indicadores de status** em tempo real

## 🏗️ Arquitetura do Sistema

### 📦 Estrutura do Projeto

```
detectoralagamento/
├── detectoralagamento.ino    # Código principal (~970 linhas)
├── README.md                 # Documentação do projeto
└── circuit_diagram/          # Diagramas de circuito (opcional)
```

### ⚙️ **Componentes Utilizados**

#### **🔌 Hardware Principal**
- **ESP32 DevKit V1**: Microcontrolador principal com WiFi integrado
- **JSN-SR04T**: Sensor ultrassônico à prova d'água (2-400cm)
- **YL-83**: Sensor de chuva com saída analógica
- **Buzzer Passivo**: Alarme sonoro para alertas
- **5x LEDs**: Indicadores visuais de status (cores específicas)
- **Resistores**: 220Ω para proteção dos LEDs

#### **📍 Pinagem Detalhada**
```cpp
// Sensores
const int trigPin = 16;      // JSN-SR04T Trigger
const int echoPin = 17;      // JSN-SR04T Echo  
const int rainSensorPin = 36; // YL-83 Analógico
const int buzzerPin = 12;    // Buzzer

// LEDs Indicadores
const int LED_AZUL = 4;      // Internet
const int LED_BRANCO = 5;    // Telegram
const int LED_VERMELHO = 18; // Perigo
const int LED_AMARELO = 32;  // Alerta
const int LED_VERDE = 21;    // Normal
```

## 🛠️ Tecnologias e Bibliotecas

### ⚙️ **Core Technologies**
- **Arduino IDE**: Ambiente de desenvolvimento
- **ESP32**: Microcontrolador com WiFi/Bluetooth
- **HTML5/CSS3/JavaScript**: Interface web responsiva
- **JSON**: Comunicação de dados via API

### 📚 **Bibliotecas Utilizadas**
```cpp
#include <WiFi.h>           // Conexão WiFi
#include <WebServer.h>      // Servidor HTTP
#include <HTTPClient.h>     // Cliente HTTP para Telegram
#include <WiFiClientSecure.h> // HTTPS seguro
#include <ArduinoJson.h>    // Manipulação JSON
#include <time.h>           // Sincronização NTP
```

## 🚀 Como Configurar o Projeto

### 📋 Pré-requisitos

- **Arduino IDE 1.8.19+** instalado
- **Biblioteca ESP32** configurada no Arduino IDE
- **Bibliotecas adicionais**: ArduinoJson (v6.x)
- **Bot do Telegram** criado via @BotFather
- **ID do grupo** do Telegram para notificações

### 💻 Passos de Instalação

#### 1. **Configuração do Arduino IDE**
```bash
# Adicione a URL do ESP32 no Gerenciador de Placas:
https://dl.espressif.com/dl/package_esp32_index.json

# Instale a biblioteca ArduinoJson via Library Manager
```

#### 2. **Configuração das Credenciais**
```cpp
// Edite no código principal:
const char* ssid = "SEU_WIFI_AQUI";
const char* password = "SUA_SENHA_AQUI";

const String BOT_TOKEN = "SEU_BOT_TOKEN_AQUI";
const String GROUP_CHAT_ID = "SEU_GROUP_ID_AQUI";
```

#### 3. **Criação do Bot Telegram**
1. Converse com @BotFather no Telegram
2. Digite `/newbot` e siga as instruções
3. Salve o **token** fornecido
4. Adicione o bot ao seu grupo
5. Obtenha o **Group ID** (use @get_id_bot)

#### 4. **Upload do Código**
```bash
# Selecione a placa: ESP32 Dev Module
# Configure a velocidade: 115200 baud
# Faça o upload do código
```

### 🔧 Configuração dos Limiares

```cpp
// Personalize os limiares no código:
const float DANGER_THRESHOLD = 30.0;   // 30cm (perigo)
const float WARNING_THRESHOLD = 50.0;  // 50cm (alerta)
const float HYSTERESIS = 5.0;          // 5cm (histerese)
const int RAIN_THRESHOLD = 3000;       // Valor analógico chuva
```

## 🎨 Interface e Funcionalidades

### 🖥️ **Dashboard Web**

A interface web é acessível pelo IP do ESP32 e oferece:

#### **📊 Informações em Tempo Real**
- **Status atual** com indicador colorido animado
- **Distância medida** pelo sensor ultrassônico
- **Status da chuva** com valor analógico
- **Horário sincronizado** via NTP
- **Uptime do sistema** formatado

#### **💡 Painel de LEDs Virtual**
```html
<!-- Indicadores visuais que refletem o hardware -->
🔵 Internet    ⚪ Telegram    🔴 Perigo    🟡 Alerta    🟢 Normal
```

#### **📜 Histórico de Eventos**
- **Log circular** com 20 entradas máximo
- **Timestamps precisos** (horário real + uptime)
- **Cores diferenciadas** por tipo de evento
- **Atualização automática** sem refresh

### 📱 **Notificações Telegram**

#### **🚨 Mensagem de Perigo**
```
🚨 ALERTA CRÍTICO!

Nível: 25.3cm
Status: PERIGO! Nível crítico
Horário: 15/01/2025 14:30:25
LEDs: Vermelho piscando

AÇÃO IMEDIATA NECESSÁRIA!
```

#### **⚠️ Mensagem de Alerta**
```
⚠️ ALERTA DE NÍVEL!

Nível: 45.8cm
Status: ALERTA! Nível elevado + Chuva
Horário: 15/01/2025 14:25:10
LEDs: Amarelo piscando

Monitore a situação!
```

#### **✅ Mensagem de Normalização**
```
✅ SITUAÇÃO NORMALIZADA

Nível: 65.2cm
Status: Normal
Horário: 15/01/2025 14:35:45
LEDs: Verde fixo

Sistema sob controle!
```

## 🧪 Funcionalidades Detalhadas

### 🔍 **Sistema de Medição**

#### **📏 Sensor Ultrassônico (JSN-SR04T)**
```cpp
float measureDistance() {
  // Trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Mede tempo de resposta com timeout
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration > 0) {
    float dist = duration * 0.034 / 2;
    if (dist >= 2 && dist <= 400) { // Validação da faixa
      return dist;
    }
  }
  
  return distance; // Retorna último valor válido
}
```

#### **☔ Sensor de Chuva (YL-83)**
```cpp
// Leitura analógica invertida (baixo = chuva)
int rainAnalogValue = analogRead(rainSensorPin);
isRaining = rainAnalogValue < RAIN_THRESHOLD; // 3000 padrão
```

### ⚡ **Sistema Não-Bloqueante**

O código utiliza `millis()` em vez de `delay()` para operação contínua:

```cpp
// Timers para controle de intervalos
unsigned long lastMeasurement = 0;
unsigned long lastDebugPrint = 0;
unsigned long lastTelegramSent = 0;
unsigned long lastLEDUpdate = 0;

// Intervalos configuráveis
const unsigned long MEASUREMENT_INTERVAL = 2000;  // 2s
const unsigned long DEBUG_INTERVAL = 10000;       // 10s
const unsigned long TELEGRAM_COOLDOWN = 300000;   // 5min
const unsigned long LED_UPDATE_INTERVAL = 50;     // 50ms
```

### 🔄 **Sistema de Histerese**

Para evitar oscilações entre estados:

```cpp
// Estado atual vs anterior com margem de tolerância
if (distance <= DANGER_THRESHOLD) {
  alertStatus = "🚨 PERIGO! Nível crítico";
} 
else if (distance <= DANGER_THRESHOLD + HYSTERESIS && 
         alertStatus.indexOf("PERIGO") >= 0) {
  // Mantém estado de perigo até ultrapassar histerese
  alertStatus = "🚨 PERIGO! Nível crítico";
}
```

## 📈 Características Avançadas

### 🕐 **Sincronização de Horário**
```cpp
// Configuração NTP para Brasília (GMT-3)
const long gmtOffset_sec = -3 * 3600;
const int daylightOffset_sec = 0;

// Sincronização segura com fallback
configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
```

### 🔊 **Alarmes Sonoros Diferenciados**
```cpp
void triggerAlarmSound(bool continuous) {
  if (continuous) {
    tone(buzzerPin, 1000); // Som contínuo (perigo)
  } else {
    // Som intermitente (alerta) - 1s on/off
    static bool buzzerState = false;
    if (millis() - lastBuzzerUpdate >= 1000) {
      buzzerState = !buzzerState;
      if (buzzerState) {
        tone(buzzerPin, 800);
      } else {
        noTone(buzzerPin);
      }
      lastBuzzerUpdate = millis();
    }
  }
}
```

### 📊 **API REST Simples**
```cpp
// Endpoint /api/data retorna JSON
{
  "status": "✅ Normal",
  "distance": "65.2",
  "rain": "☀️ Sem chuva",
  "rainAnalog": 4095,
  "time": "15/01/2025 14:30:25",
  "uptime": "2h 15m",
  "telegramStatus": "✅ Conectado",
  "statusClass": "normal",
  "ledInternet": "on",
  "ledTelegram": "on",
  "ledPerigo": "off",
  "ledAlerta": "off",
  "ledNormal": "on",
  "logEntries": "<div class='log-entry'>...</div>"
}
```

## 🚨 Sistema de Alertas

### 📋 **Níveis de Alerta**

| Nível | Distância | LED | Som | Telegram | Ação |
|-------|-----------|-----|-----|----------|------|
| **Normal** | > 55cm | 🟢 Fixo | Silêncio | - | Monitoramento |
| **Alerta** | 30-50cm | 🟡 Pisca | Intermitente | ⚠️ | Atenção |
| **Perigo** | < 30cm | 🔴 Pisca | Contínuo | 🚨 | Ação Imediata |

### 🎯 **Lógica de Decisão**
```cpp
// Considera chuva como fator agravante
if (distance <= DANGER_THRESHOLD) {
  alertStatus = "🚨 PERIGO! Nível crítico";
  if (isRaining) alertStatus += " + Chuva";
  triggerAlarmSound(true);
} 
else if (distance <= WARNING_THRESHOLD) {
  alertStatus = "⚠️ ALERTA! Nível elevado";
  if (isRaining) alertStatus += " + Chuva";
  triggerAlarmSound(false);
}
else {
  alertStatus = "✅ Normal";
  if (isRaining) alertStatus += " (Chovendo)";
  noTone(buzzerPin);
}
```

## 🔧 Personalização e Expansões

### ⚙️ **Configurações Principais**
```cpp
// Ajuste conforme necessidade
const float DANGER_THRESHOLD = 30.0;   // Nível crítico
const float WARNING_THRESHOLD = 50.0;  // Nível de alerta
const float HYSTERESIS = 5.0;          // Margem de tolerância
const int RAIN_THRESHOLD = 3000;       // Sensibilidade chuva
const unsigned long TELEGRAM_COOLDOWN = 300000; // Cooldown notificações
```

### 🚀 **Possíveis Melhorias**

#### **📡 Conectividade**
- [ ] Backup de rede via hotspot móvel
- [ ] Integração com MQTT para IoT
- [ ] Notificações por email (SMTP)
- [ ] API para integração com sistemas externos

#### **📊 Dados e Análise**
- [ ] Banco de dados local (SPIFFS/LittleFS)
- [ ] Gráficos históricos em tempo real
- [ ] Exportação de dados CSV
- [ ] Previsão de tendências com IA

#### **🔒 Segurança**
- [ ] Autenticação na interface web
- [ ] Criptografia de comunicação
- [ ] Backup em nuvem seguro
- [ ] Logs de auditoria

#### **⚡ Hardware**
- [ ] Sensor de temperatura/umidade
- [ ] Painel solar para autonomia
- [ ] Display OLED local
- [ ] Sensores redundantes

## 📊 Especificações Técnicas

### 🔌 **Requisitos de Energia**
- **Tensão**: 5V DC (via USB ou fonte externa)
- **Consumo médio**: ~150mA (WiFi ativo)
- **Consumo pico**: ~300mA (durante transmissão)

### 📡 **Conectividade**
- **WiFi**: 802.11 b/g/n (2.4GHz)
- **Protocolo**: HTTP/HTTPS
- **API**: Telegram Bot API
- **Sincronização**: NTP (pool.ntp.org)

### 📏 **Precisão dos Sensores**
- **JSN-SR04T**: ±1cm (faixa 20cm-4m)
- **YL-83**: Analógico 0-4095 (12-bit ADC)
- **Intervalo de medição**: 2 segundos
- **Timeout de leitura**: 30ms

## 👥 Informações do Projeto

### 🎓 **Desenvolvedores**
- **Nome**: Andryus Zolet, Nicolas Medeiros e Valentin Rojas
- **GitHub**: [@Andryus Zolet](https://github.com/AndryusZolet), [@Nicolas Medeiros](https://github.com/Nicolas-Medeiros), [@Valentin-Rojas](https://github.com/Valentin-Rojas)
- **Projeto**: Sistema IoT de Monitoramento de Alagamentos

### 🏷️ **Versão**
- **Versão atual**: 1.0.0

### 📜 **Licença Acadêmica**
Este projeto foi desenvolvido exclusivamente para fins educacionais como parte do aprendizado de **Performance em Sistemas Ciberfísicos**. O código é aberto para consulta e aprendizado.

### 📞 **Suporte**
- **Issues**: [GitHub Issues](https://github.com/Valentin-Rojas/detectoralagamento/issues)
- **Documentação**: README.md incluído no repositório

---

<div align="center">
  <h3>🌊 Sistema IoT para Prevenção de Alagamentos</h3>
  <p>
    <strong>Detector de Alagamento ESP32</strong><br>
    Monitoramento inteligente com alertas automáticos via Telegram
  </p>
  
  ⭐ **Tecnologia IoT aplicada à segurança e prevenção** ⭐
</div>

---

## 📊 Status do Projeto

![Status](https://img.shields.io/badge/Status-Ativo-brightgreen?style=for-the-badge)
![ESP32](https://img.shields.io/badge/ESP32-Compatible-blue?style=for-the-badge)
![IoT](https://img.shields.io/badge/IoT-Ready-orange?style=for-the-badge)
![Telegram](https://img.shields.io/badge/Telegram-Bot-26A5E4?style=for-the-badge)

**Projeto Acadêmico Desenvolvido no 4 período**: - Primeiro semestre de 2025 (2025/1)
