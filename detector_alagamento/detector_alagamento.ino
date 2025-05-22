#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// Configurações WiFi
const char* ssid = "Visitantes";
const char* password = "01619041901";

// Pinos dos sensores
const int trigPin = 23;
const int echoPin = 22;
const int rainSensorPin = 21;
const int buzzerPin = 12;

// Variáveis de medição
float distance = 0;
bool isRaining = false;
String alertStatus = "Normal";

// Cria o servidor web na porta 80
WebServer server(80);

// Página HTML
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
      background-color: #f5f5f5;
    }
    .container { 
      max-width: 600px; 
      margin: 0 auto; 
      background: white;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    .status-box { 
      padding: 20px; 
      margin: 10px 0; 
      border-radius: 10px; 
      color: white;
      font-weight: bold;
    }
    .normal { background-color: #4CAF50; }
    .warning { background-color: #FFC107; color: #333; }
    .danger { background-color: #F44336; }
    .data { 
      margin: 20px 0; 
      font-size: 24px;
      color: #333;
    }
    .rain { 
      margin-top: 30px; 
      padding: 10px;
      background-color: #e3f2fd;
      border-radius: 5px;
    }
    h1 {
      color: #2c3e50;
      margin-bottom: 30px;
    }
    .last-update {
      font-size: 14px;
      color: #7f8c8d;
      margin-top: 20px;
    }
  </style>
  <meta http-equiv="refresh" content="5">
</head>
<body>
  <div class="container">
    <h1>🌧️ Monitor de Alagamentos 🚨</h1>
    
    <div class="status-box %STATUS_CLASS%">
      <h2>Status: %STATUS%</h2>
    </div>
    
    <div class="data">
      <p>📏 Nível da água: <strong>%DISTANCE% cm</strong></p>
      <div class="rain">
        <p>☔ Condição de chuva: <strong>%RAIN%</strong></p>
      </div>
    </div>
    
    <p class="last-update">⌚ Última atualização: %TIME%</p>
  </div>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  
  // Configura pinos
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(rainSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  // Conecta WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  // Configura rotas do servidor web
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  
  // Atualiza medições
  distance = measureDistance();
  isRaining = digitalRead(rainSensorPin) == LOW;
  
  // Determina status
  if (distance < 30.0) {
    alertStatus = "Perigo! Nível crítico";
    if (!isRaining) triggerAlarmSound();
  } 
  else if (distance < 50.0 && isRaining) {
    alertStatus = "Alerta! Nível elevado e chovendo";
    triggerAlarmSound();
  }
  else {
    alertStatus = "Normal";
  }
  
  delay(1000);
}

void handleRoot() {
  String page = htmlPage;
  
  // Substitui placeholders pelos valores atuais
  page.replace("%DISTANCE%", String(distance));
  page.replace("%RAIN%", isRaining ? "Chovendo" : "Sem chuva");
  page.replace("%TIME%", getTime());
  page.replace("%STATUS%", alertStatus);
  
  // Define a classe CSS baseada no status
  if (alertStatus.indexOf("Perigo") >= 0) {
    page.replace("%STATUS_CLASS%", "danger");
  } 
  else if (alertStatus.indexOf("Alerta") >= 0) {
    page.replace("%STATUS_CLASS%", "warning");
  } 
  else {
    page.replace("%STATUS_CLASS%", "normal");
  }
  
  server.send(200, "text/html", page);
}

String getTime() {
  // Retorna a hora atual (simplificado)
  return String(millis() / 1000) + " segundos atrás";
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

void triggerAlarmSound() {
  tone(buzzerPin, 1000, 500);
}