#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ==========================================
// CONFIGURAÇÕES DE REDE E MQTT
// ==========================================
const char* WIFI_SSID     = "SUA_REDE_WIFI";       // Substiua pelo seu Wi-Fi
const char* WIFI_PASSWORD = "SUA_SENHA_WIFI";      // Substitua pela sua senha
const char* MQTT_BROKER   = "broker.hivemq.com";
const int   MQTT_PORT     = 1883;                  // Porta TCP padrão (sem SSL)

// Tópicos configurados de acordo com o Dashboard HTML
const char* TOPICO_TELEMETRIA = "estufa/senai/dados";   // ESP32 publica para o Web Dashboard
const char* TOPICO_COMANDO    = "estufa/senai/comando"; // Web Dashboard envia ordens para o ESP32

// ==========================================
// MAPEAMENTO DE PINOS (ESP32-S3-N16R8)
// ==========================================
#define DHTPIN        15      // Pino do sensor DHT
#define DHTTYPE       DHT11   // Troque para DHT22 se necessário
#define LDR_PIN       4       // Sensor de Luminosidade (ADC1_3)
#define POT_PIN       5       // Reservatório/Solo (ADC1_4)
#define LED_IRRIGACAO 6       // Atuador da Bomba
#define BUZZER_PIN    7       // Atuador de Alerta Exaustão

#define I2C_SDA       8       // SDA Hardware conforme o pinout do ESP32-S3
#define I2C_SCL       9       // SCL Hardware conforme o pinout do ESP32-S3

// ==========================================
// PARÂMETROS E OBJETOS DO SISTEMA
// ==========================================
#define TEMP_LIMITE_ALTA 30.0
#define SOLO_CRITICO     300

WiFiClient espClient;
PubSubClient mqttClient(espClient);
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// Variáveis Globais de Estado
bool bombaManualWeb = false;
unsigned long tempoUltimoEnvio = 0;

// Protótipo de Funções
void conectarWiFi();
void conectarMQTT();
void callbackMQTT(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Estufa Agrícola ESP32-S3 (IoT Conectada) ---");

  pinMode(LED_IRRIGACAO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_IRRIGACAO, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(I2C_SDA, I2C_SCL);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Estufa IoT S3");
  lcd.setCursor(0, 1);
  lcd.print("Conectando...");

  dht.begin();
  conectarWiFi();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callbackMQTT);

  lcd.clear();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop(); // Processa mensagens MQTT recebidas

  // Envio periódico de dados a cada 2 segundos
  if (millis() - tempoUltimoEnvio >= 2000) {
    tempoUltimoEnvio = millis();

    // 1. Leitura dos sensores físicos
    float temp = dht.readTemperature();
    float umid = dht.readHumidity();
    int leituraLDR = analogRead(LDR_PIN);
    int leituraSolo = analogRead(POT_PIN);

    if (isnan(temp) || isnan(umid)) {
      Serial.println("Falha na leitura do sensor DHT!");
      return;
    }

    // Conversões de escala
    int pctSolo = map(leituraSolo, 0, 4095, 0, 100);
    int pctLuz  = map(leituraLDR, 0, 4095, 0, 100);

    // 2. Lógica de Automação Local com override Web
    // A bomba liga se o solo estiver crítico OU se for acionada manualmente no Dashboard
    bool estadoBomba = (leituraSolo < SOLO_CRITICO) || bombaManualWeb;
    digitalWrite(LED_IRRIGACAO, estadoBomba ? HIGH : LOW);

    // Alarme/Exaustor por temperatura
    bool estadoAlarme = (temp > TEMP_LIMITE_ALTA);
    digitalWrite(BUZZER_PIN, estadoAlarme ? HIGH : LOW);

    // 3. Atualização do Display LCD
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temp, 1);
    lcd.print((char)223);
    lcd.print("C U:");
    lcd.print(umid, 0);
    lcd.print("%   ");

    lcd.setCursor(0, 1);
    lcd.print("Solo:");
    lcd.print(pctSolo);
    lcd.print("% L:");
    lcd.print(pctLuz);
    lcd.print("%   ");

    // 4. Montagem e Publicação da Telemetria via JSON (Topico: estufa/senai/dados)
    StaticJsonDocument<200> doc;
    doc["temp"]  = serialized(String(temp, 1));
    doc["umid"]  = (int)umid;
    doc["luz"]   = pctLuz;
    doc["agua"]  = pctSolo;
    doc["bomba"] = estadoBomba;

    char bufferJSON[200];
    serializeJson(doc, bufferJSON);

    mqttClient.publish(TOPICO_TELEMETRIA, bufferJSON);
    Serial.printf("[TX MQTT] %s\n", bufferJSON);
  }
}

// ==========================================
// FUNÇÕES AUXILIARES DE CONEXÃO E CALLBACK
// ==========================================

void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado! IP: " + WiFi.localIP().toString());
}

void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Tentando conexão MQTT com broker...");
    String clientId = "ESP32S3_Estufa_Client_" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" Conectado!");
      mqttClient.subscribe(TOPICO_COMANDO); // Escuta comandos vindos da Web
    } else {
      Serial.print(" Falhou, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando em 5 segundos...");
      delay(5000);
    }
  }
}

// Executada automaticamente quando o Dashboard envia um JSON para "estufa/senai/comando"
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  Serial.print("[RX MQTT] ");
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("Erro ao decodificar JSON: ");
    Serial.println(error.f_str());
    return;
  }

  // Atualiza o estado da bomba a partir do comando da interface web
  if (doc.containsKey("bomba")) {
    bombaManualWeb = doc["bomba"];
    digitalWrite(LED_IRRIGACAO, bombaManualWeb ? HIGH : LOW);
    Serial.printf("Comando recebido - Bomba Alterada via Web para: %s\n", bombaManualWeb ? "LIGADA" : "DESLIGADA");
  }
}