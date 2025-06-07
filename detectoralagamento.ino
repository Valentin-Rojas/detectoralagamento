#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>

// Configurações WiFi
const char* ssid = "ValeDesktop";
const char* password = "12ValeDesktop34";

// Configurações do Bot Telegram para GRUPO
const String BOT_TOKEN = "SEU_BOT_TOKEN_AQUI";    // Token do seu bot
const String GROUP_CHAT_ID = "SEU_GROUP_ID_AQUI"; //
const String TELEGRAM_URL = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage";

// Pinos dos sensores
const int trigPin = 16;      // Trig do JSN-SR04T conectado no GPIO16
const int echoPin = 17;      // Echo do JSN-SR04T conectado no GPIO17
const int rainSensorPin = 36; // AO do YL-83 conectado no GPIO36
const int buzzerPin = 12;

// Limiares e configurações
const float DANGER_THRESHOLD = 30.0;   // 30cm (nível crítico)
const float WARNING_THRESHOLD = 50.0;  // 50cm (nível de alerta)
const float HYSTERESIS = 5.0;          // 5cm de histerese
const int MAX_LOG_ENTRIES = 20;        // Máximo de entradas no log
const int RAIN_THRESHOLD = 3000;       // Limiar analógico para chuva (ajuste conforme necessário)

// Variáveis de medição
float distance = 0;
bool isRaining = false;
String alertStatus = "Normal";
String previousStatus = "";

// Controle do Telegram
unsigned long lastTelegramSent = 0;
const unsigned long TELEGRAM_COOLDOWN = 300000; // 5 minutos entre mensagens do mesmo tipo
String lastTelegramStatus = "";

// Controle de medições e debug
unsigned long lastMeasurement = 0;
unsigned long lastDebugPrint = 0;
const unsigned long MEASUREMENT_INTERVAL = 2000;  // 2 segundos
const unsigned long DEBUG_INTERVAL = 10000;       // 10 segundos para debug

// Configuração de fuso horário (Brasília GMT-3)
const long gmtOffset_sec = -3 * 3600;  // -3 horas em segundos
const int daylightOffset_sec = 0;      // Sem horário de verão

// Sistema de log
String logEntries[MAX_LOG_ENTRIES];
int logIndex = 0;

// Cria o servidor web na porta 80
WebServer server(80);

// Página HTML com atualização AJAX mais rápida
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Monitor de Alagamentos</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { 
      font-family: 'Segoe UI', Arial, sans-serif; 
      text-align: center; 
      margin: 0; 
      padding: 20px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
    }
    .container { 
      max-width: 800px; 
      margin: 0 auto; 
      background: rgba(255,255,255,0.95);
      padding: 30px;
      border-radius: 15px;
      box-shadow: 0 8px 32px rgba(0,0,0,0.1);
      backdrop-filter: blur(10px);
    }
    .status-box { 
      padding: 25px; 
      margin: 20px 0; 
      border-radius: 15px; 
      color: white;
      font-weight: bold;
      font-size: 20px;
      transition: all 0.3s ease;
    }
    .normal { 
      background: linear-gradient(45deg, #4CAF50, #45a049);
      box-shadow: 0 4px 15px rgba(76, 175, 80, 0.3);
    }
    .warning { 
      background: linear-gradient(45deg, #FFC107, #ffb300);
      color: #333;
      box-shadow: 0 4px 15px rgba(255, 193, 7, 0.3);
    }
    .danger { 
      background: linear-gradient(45deg, #F44336, #d32f2f);
      box-shadow: 0 4px 15px rgba(244, 67, 54, 0.3);
      animation: pulse 2s infinite;
    }
    @keyframes pulse {
      0%, 100% { transform: scale(1); }
      50% { transform: scale(1.02); }
    }
    .sensor-card {
      background: #f8f9fa;
      border-radius: 10px;
      padding: 20px;
      margin: 15px 0;
      border-left: 4px solid #007bff;
      transition: transform 0.2s ease;
    }
    .sensor-card:hover {
      transform: translateY(-2px);
    }
    .rain-card {
      border-left-color: #17a2b8;
    }
    .value {
      font-size: 28px;
      font-weight: bold;
      color: #007bff;
      margin: 10px 0;
    }
    h1 {
      color: #2c3e50;
      margin-bottom: 30px;
      font-size: 32px;
    }
    .last-update {
      font-size: 14px;
      color: #6c757d;
      margin-top: 20px;
      padding: 10px;
      background: #e9ecef;
      border-radius: 5px;
    }
    .telegram-status {
      background: #e3f2fd;
      border-left: 4px solid #2196F3;
      padding: 10px;
      margin: 15px 0;
      border-radius: 5px;
      font-size: 14px;
    }
    .log-container {
      margin-top: 30px;
      text-align: left;
      border-top: 2px solid #dee2e6;
      padding-top: 20px;
      max-height: 400px;
      overflow-y: auto;
    }
    .log-title {
      font-size: 20px;
      color: #2c3e50;
      margin-bottom: 15px;
      font-weight: bold;
    }
    .log-entry {
      padding: 12px;
      border-bottom: 1px solid #eee;
      font-size: 14px;
      border-radius: 5px;
      margin-bottom: 5px;
      background: #f8f9fa;
    }
    .log-time {
      color: #6c757d;
      font-weight: bold;
    }
    .log-normal { border-left: 4px solid #4CAF50; }
    .log-warning { border-left: 4px solid #FFC107; }
    .log-danger { border-left: 4px solid #F44336; }
    /* Removido efeito de loading roxo */
  </style>
</head>
<body>
  <div class="container">
    <h1>🌧️ Monitor de Alagamentos 🚨</h1>
    
    <div class="status-box %STATUS_CLASS%" id="statusBox">
      <h2>Status: <span id="statusText">%STATUS%</span></h2>
    </div>
    
    <div class="sensor-card">
      <h3>📏 Sensor de Nível</h3>
      <div class="value" id="distanceValue">%DISTANCE% cm</div>
      <small>Distância até a superfície da água</small>
    </div>
    
    <div class="sensor-card rain-card">
      <h3>☔ Sensor de Chuva</h3>
      <div class="value" id="rainValue">%RAIN%</div>
      <small>Valor analógico: <span id="rainAnalog">%RAIN_ANALOG%</span> | Limiar: %RAIN_THRESHOLD%</small>
    </div>
    
    <div class="telegram-status">
      📱 <strong>Telegram Grupo:</strong> <span id="telegramStatus">%TELEGRAM_STATUS%</span>
    </div>
    
    <p class="last-update">
      ⌚ Última atualização: <span id="lastUpdate">%TIME%</span><br>
      🔧 Uptime: <span id="uptime">%UPTIME%</span>
    </p>
    
    <div class="log-container">
      <div class="log-title">📜 Histórico de Eventos:</div>
      <div id="logEntries">%LOG_ENTRIES%</div>
    </div>
  </div>

  <script>
    // Atualização AJAX a cada 3 segundos (sem efeito visual)
    function updateData() {
      fetch('/api/data')
        .then(response => response.json())
        .then(data => {
          // Atualiza valores na tela
          document.getElementById('statusText').textContent = data.status;
          document.getElementById('distanceValue').textContent = data.distance + ' cm';
          document.getElementById('rainValue').textContent = data.rain;
          document.getElementById('rainAnalog').textContent = data.rainAnalog;
          document.getElementById('telegramStatus').textContent = data.telegramStatus;
          document.getElementById('lastUpdate').textContent = data.time;
          document.getElementById('uptime').textContent = data.uptime;
          
          // Atualiza classe do status
          const statusBox = document.getElementById('statusBox');
          statusBox.className = 'status-box ' + data.statusClass;
          
          // Atualiza logs
          document.getElementById('logEntries').innerHTML = data.logEntries;
        })
        .catch(error => {
          console.error('Erro ao atualizar dados:', error);
        });
    }
    
    // Atualiza a cada 3 segundos
    setInterval(updateData, 3000);
    
    // Primeira atualização após 1 segundo
    setTimeout(updateData, 1000);
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial.println("=== Monitor de Alagamentos com Telegram ===");
  
  // Configura pinos
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(rainSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  // Teste inicial do buzzer
  tone(buzzerPin, 1000, 200);
  delay(300);
  noTone(buzzerPin);
  
  // Conecta WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("✅ Conectado! IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 Força do sinal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // Configura horário via NTP (versão segura)
    Serial.print("🕐 Sincronizando horário com NTP...");
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
    
    // Aguarda sincronização de forma segura
    struct tm timeinfo;
    int attempts = 0;
    bool timeSet = false;
    
    while (attempts < 15 && !timeSet) {
      if (getLocalTime(&timeinfo)) {
        timeSet = true;
        Serial.println(" ✅ Horário sincronizado!");
        Serial.println("🕐 Horário atual: " + getRealTime());
      } else {
        Serial.print(".");
        delay(1000);
        attempts++;
      }
    }
    
    if (!timeSet) {
      Serial.println(" ⚠️ Timeout na sincronização - usando uptime");
    }
    
    // Teste do bot Telegram
    String initMessage = "🚀 SISTEMA DE MONITORAMENTO INICIADO!\n\n";
    initMessage += "📍 Localização: Monitor de Alagamentos\n";
    initMessage += "🌐 IP do sistema: " + WiFi.localIP().toString() + "\n";
    initMessage += "⏰ Horário de inicialização: " + getRealTime() + "\n";
    initMessage += "📊 Status inicial: Normal\n\n";
    initMessage += "✅ Sistema online e monitorando!";
    sendTelegramMessage(initMessage);
  } else {
    Serial.println("❌ Falha na conexão WiFi!");
  }

  // Inicializa o log
  addLogEntry("Sistema iniciado. Status inicial: Normal");
  
  // Configura rotas do servidor web
  server.on("/", handleRoot);
  server.on("/api/data", handleApiData); // Nova rota para dados JSON
  server.begin();
  
  Serial.println("🌐 Servidor web iniciado!");
  Serial.println("=========================");
}

void loop() {
  server.handleClient();
  
  unsigned long currentTime = millis();
  
  // Atualiza medições em intervalo controlado
  if (currentTime - lastMeasurement >= MEASUREMENT_INTERVAL) {
    lastMeasurement = currentTime;
    
    // Medições dos sensores
    distance = measureDistance();
    int rainAnalogValue = analogRead(rainSensorPin);
    
    // Corrige detecção de chuva: valor BAIXO = chuva
    isRaining = rainAnalogValue < RAIN_THRESHOLD;
    
    // Debug detalhado a cada 10 segundos
    if (currentTime - lastDebugPrint >= DEBUG_INTERVAL) {
      lastDebugPrint = currentTime;
      Serial.println("=== DEBUG SENSORES ===");
      Serial.println("Distância: " + String(distance, 1) + " cm");
      Serial.println("Chuva Analógica: " + String(rainAnalogValue));
      Serial.println("Limiar chuva: " + String(RAIN_THRESHOLD));
      Serial.println("Está chovendo: " + String(isRaining ? "SIM" : "NÃO"));
      Serial.println("Status atual: " + alertStatus);
      Serial.println("====================");
    }
    
    // Determina status com histerese CORRIGIDA
    previousStatus = alertStatus;
    
    if (distance <= DANGER_THRESHOLD) {
      alertStatus = "🚨 PERIGO! Nível crítico";
      triggerAlarmSound(true);
    } 
    else if (distance <= DANGER_THRESHOLD + HYSTERESIS && alertStatus.indexOf("PERIGO") >= 0) {
      // Mantém perigo até sair da zona de histerese
      alertStatus = "🚨 PERIGO! Nível crítico";
      triggerAlarmSound(true);
    }
    else if (distance <= WARNING_THRESHOLD) {
      alertStatus = "⚠️ ALERTA! Nível elevado";
      if (isRaining) {
        alertStatus += " + Chuva";
      }
      triggerAlarmSound(false);
    }
    else if (distance <= WARNING_THRESHOLD + HYSTERESIS && alertStatus.indexOf("ALERTA") >= 0) {
      // Mantém alerta até sair da zona de histerese
      alertStatus = "⚠️ ALERTA! Nível elevado";
      if (isRaining) {
        alertStatus += " + Chuva";
      }
      triggerAlarmSound(false);
    }
    else {
      alertStatus = "✅ Normal";
      if (isRaining) {
        alertStatus += " (Chovendo)";
      }
      noTone(buzzerPin);
    }
    
    // Registra mudança de status no log
    if (alertStatus != previousStatus) {
      String logMessage = "Status: " + previousStatus + " → " + alertStatus;
      logMessage += " | Água: " + String(distance, 1) + "cm";
      logMessage += " | Chuva: " + String(isRaining ? "Sim" : "Não");
      logMessage += " | Analog: " + String(rainAnalogValue);
      addLogEntry(logMessage);
      
      // Envia notificação por Telegram (com cooldown de 20 minutos)
      sendStatusUpdate();
    }
  }
  
  delay(100); // Delay pequeno para não sobrecarregar o sistema
}

void sendStatusUpdate() {
  unsigned long currentTime = millis();
  
  // Verifica se deve enviar mensagem (cooldown de 20 minutos para mesmo status)
  if (currentTime - lastTelegramSent < TELEGRAM_COOLDOWN && alertStatus == lastTelegramStatus) {
    Serial.println("⏰ Telegram: Aguardando cooldown (" + String((TELEGRAM_COOLDOWN - (currentTime - lastTelegramSent)) / 60000) + " min restantes)");
    return;
  }
  
  // Só envia para mudanças de status importantes
  if (alertStatus.indexOf("PERIGO") >= 0 || alertStatus.indexOf("ALERTA") >= 0 || 
      (previousStatus.indexOf("PERIGO") >= 0 || previousStatus.indexOf("ALERTA") >= 0)) {
    
    String message = "";
    
    if (alertStatus.indexOf("PERIGO") >= 0) {
      message = "🚨🆘 ALERTA CRÍTICO - ALAGAMENTO! 🆘🚨\n\n";
      message += "⚠️ NÍVEL DE ÁGUA CRÍTICO DETECTADO!\n\n";
      message += "📊 DADOS ATUAIS:\n";
      message += "🔴 Status: " + alertStatus + "\n";
      message += "📏 Nível: " + String(distance, 1) + " cm (CRÍTICO!)\n";
      message += "☔ Chuva: " + String(isRaining ? "SIM - Agravando situação" : "NÃO") + "\n";
      message += "⏰ Horário: " + getFormattedTime() + "\n\n";
      message += "🆘 AÇÃO NECESSÁRIA IMEDIATA!\n";
      message += "📞 Contate as autoridades se necessário\n";
      message += "🏃‍♂️ Evacue áreas de risco\n";
      message += "⚠️ Mantenha distância de áreas inundadas";
      
    } else if (alertStatus.indexOf("ALERTA") >= 0) {
      message = "⚠️🟡 ALERTA DE MONITORAMENTO 🟡⚠️\n\n";
      message += "📈 Nível da água está elevado!\n\n";
      message += "📊 SITUAÇÃO ATUAL:\n";
      message += "🟡 Status: " + alertStatus + "\n";
      message += "📏 Nível: " + String(distance, 1) + " cm (Elevado)\n";
      message += "☔ Chuva: " + String(isRaining ? "SIM - Situação pode piorar" : "NÃO") + "\n";
      message += "⏰ Horário: " + getFormattedTime() + "\n\n";
      message += "👁️ MANTENHA-SE ATENTO!\n";
      message += "📱 Monitore atualizações\n";
      message += "🎒 Prepare-se para possível evacuação";
      
    } else {
      // Só envia normalização se vinha de estado crítico
      if (previousStatus.indexOf("PERIGO") >= 0 || previousStatus.indexOf("ALERTA") >= 0) {
        message = "✅🟢 SITUAÇÃO NORMALIZADA 🟢✅\n\n";
        message += "😌 O nível da água voltou ao normal\n\n";
        message += "📊 STATUS ATUAL:\n";
        message += "✅ Status: " + alertStatus + "\n";
        message += "📏 Nível: " + String(distance, 1) + " cm (Normal)\n";
        message += "☔ Chuva: " + String(isRaining ? "SIM - Sem risco atual" : "NÃO") + "\n";
        message += "⏰ Horário: " + getFormattedTime() + "\n\n";
        message += "🎉 Situação sob controle!\n";
        message += "👀 Continuamos monitorando 24/7";
      } else {
        // Não envia mensagem para mudanças normais
        return;
      }
    }
    
    if (sendTelegramMessage(message)) {
      lastTelegramSent = currentTime;
      lastTelegramStatus = alertStatus;
      Serial.println("📱 Telegram: Mensagem enviada com sucesso!");
    }
  }
}

bool sendTelegramMessage(String message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi desconectado - não foi possível enviar mensagem");
    return false;
  }
  
  if (BOT_TOKEN == "SEU_BOT_TOKEN_AQUI" || GROUP_CHAT_ID == "SEU_GROUP_ID_AQUI") {
    Serial.println("⚠️ Configure o BOT_TOKEN e GROUP_CHAT_ID do Telegram");
    return false;
  }
  
  WiFiClientSecure client;
  client.setInsecure(); // Para simplicidade - em produção, use certificados
  HTTPClient http;
  
  if (http.begin(client, TELEGRAM_URL)) {
    http.addHeader("Content-Type", "application/json");
    
    // Cria JSON para envio
    DynamicJsonDocument doc(2048);
    doc["chat_id"] = GROUP_CHAT_ID;
    doc["text"] = message;
    doc["parse_mode"] = "HTML";
    doc["disable_notification"] = false; // Notifica todos no grupo
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode == 200) {
      Serial.println("✅ Mensagem enviada para o GRUPO do Telegram");
      http.end();
      return true;
    } else {
      Serial.println("❌ Erro ao enviar mensagem: " + String(httpResponseCode));
      String response = http.getString();
      Serial.println("Resposta: " + response);
    }
    
    http.end();
  } else {
    Serial.println("❌ Erro ao conectar com Telegram");
  }
  
  return false;
}

void handleRoot() {
  String page = htmlPage;
  
  // Substitui placeholders pelos valores atuais
  page.replace("%DISTANCE%", String(distance, 1));
  page.replace("%RAIN%", isRaining ? "🌧️ Chovendo" : "☀️ Sem chuva");
  page.replace("%RAIN_ANALOG%", String(analogRead(rainSensorPin)));
  page.replace("%TIME%", getRealTime());
  page.replace("%UPTIME%", getUptime());
  page.replace("%STATUS%", alertStatus);
  page.replace("%TELEGRAM_STATUS%", getTelegramStatus());
  page.replace("%RAIN_THRESHOLD%", String(RAIN_THRESHOLD));
  
  // Define a classe CSS baseada no status
  String statusClass = "normal";
  if (alertStatus.indexOf("PERIGO") >= 0) {
    statusClass = "danger";
  } 
  else if (alertStatus.indexOf("ALERTA") >= 0) {
    statusClass = "warning";
  }
  page.replace("%STATUS_CLASS%", statusClass);
  
  // Gera as entradas de log para a página
  page.replace("%LOG_ENTRIES%", generateLogHtml());
  
  server.send(200, "text/html", page);
}

void handleApiData() {
  // Resposta JSON para atualização AJAX
  DynamicJsonDocument doc(2048);
  
  doc["status"] = alertStatus;
  doc["distance"] = String(distance, 1);
  doc["rain"] = isRaining ? "🌧️ Chovendo" : "☀️ Sem chuva";
  doc["rainAnalog"] = analogRead(rainSensorPin);
  doc["time"] = getRealTime();
  doc["uptime"] = getUptime();
  doc["telegramStatus"] = getTelegramStatus();
  
  // Define classe CSS
  String statusClass = "normal";
  if (alertStatus.indexOf("PERIGO") >= 0) {
    statusClass = "danger";
  } 
  else if (alertStatus.indexOf("ALERTA") >= 0) {
    statusClass = "warning";
  }
  doc["statusClass"] = statusClass;
  
  doc["logEntries"] = generateLogHtml();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  server.send(200, "application/json", jsonString);
}

String generateLogHtml() {
  String logHtml = "";
  for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
    int idx = (logIndex + MAX_LOG_ENTRIES - 1 - i) % MAX_LOG_ENTRIES;
    if (logEntries[idx].length() > 0) {
      String entryClass = "log-entry";
      if (logEntries[idx].indexOf("PERIGO") >= 0) entryClass += " log-danger";
      else if (logEntries[idx].indexOf("ALERTA") >= 0) entryClass += " log-warning";
      else if (logEntries[idx].indexOf("Normal") >= 0) entryClass += " log-normal";
      
      logHtml += "<div class='" + entryClass + "'>" + logEntries[idx] + "</div>";
    }
  }
  return logHtml;
}

String getTelegramStatus() {
  if (BOT_TOKEN == "SEU_BOT_TOKEN_AQUI") {
    return "❌ Bot não configurado";
  }
  
  if (GROUP_CHAT_ID == "SEU_GROUP_ID_AQUI") {
    return "❌ Grupo não configurado";
  }
  
  unsigned long timeSinceLastMessage = millis() - lastTelegramSent;
  if (timeSinceLastMessage < 60000) { // Menos de 1 minuto
    return "✅ Ativo (enviou há " + String(timeSinceLastMessage / 1000) + "s)";
  } else if (lastTelegramSent > 0) {
    return "✅ Conectado (último: " + String(timeSinceLastMessage / 60000) + "m atrás)";
  } else {
    return "✅ Conectado (cooldown: 5min)";
  }
}

void addLogEntry(String message) {
  String timestamp = getRealTime();
  String fullMessage = "[" + timestamp + "] " + message;
  Serial.println("📝 " + fullMessage);
  
  logEntries[logIndex] = fullMessage;
  logIndex = (logIndex + 1) % MAX_LOG_ENTRIES;
}

String getFormattedTime() {
  unsigned long totalSeconds = millis() / 1000;
  unsigned long hours = (totalSeconds / 3600) % 24;
  unsigned long minutes = (totalSeconds / 60) % 60;
  unsigned long seconds = totalSeconds % 60;
  
  String timeStr = "";
  if (hours < 10) timeStr += "0";
  timeStr += String(hours) + ":";
  if (minutes < 10) timeStr += "0";
  timeStr += String(minutes) + ":";
  if (seconds < 10) timeStr += "0";
  timeStr += String(seconds);
  
  return timeStr;
}

String getUptime() {
  unsigned long totalSeconds = millis() / 1000;
  unsigned long days = totalSeconds / 86400;
  unsigned long hours = (totalSeconds % 86400) / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  
  String uptime = "";
  if (days > 0) uptime += String(days) + "d ";
  if (hours > 0) uptime += String(hours) + "h ";
  uptime += String(minutes) + "m";
  
  return uptime;
}

String getRealTime() {
  struct tm timeinfo;
  
  // Verifica se consegue obter horário de forma segura
  if (getLocalTime(&timeinfo)) {
    // Valida se a data faz sentido (ano > 2020)
    if (timeinfo.tm_year + 1900 > 2020) {
      char buffer[30];
      strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
      return String(buffer);
    }
  }
  
  // Fallback para uptime se NTP não funcionar
  return getFormattedTime();
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000); // Timeout 30ms
  if (duration > 0) {
    float dist = duration * 0.034 / 2;
    if (dist >= 2 && dist <= 400) { // Faixa válida
      return dist;
    }
  }
  
  return distance; // Retorna último valor válido se erro
}

void triggerAlarmSound(bool continuous) {
  if (continuous) {
    tone(buzzerPin, 1000);
  } else {
    unsigned long currentTime = millis();
    if ((currentTime % 2000) < 1000) {
      tone(buzzerPin, 800);
    } else {
      noTone(buzzerPin);
    }
  }
}