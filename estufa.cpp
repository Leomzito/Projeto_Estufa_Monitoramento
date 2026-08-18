#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ==========================================
// DEFINIÇÃO DOS PINOS AJUSTADOS PARA SEU PINOUT ESP32-S3
// ==========================================
#define DHTPIN        15      // Digital I/O
#define DHTTYPE       DHT11   // DHT 22 Ou DHT11
#define LDR_PIN       4       // Canal ADC1_3 (Seguro com Wi-Fi)
#define POT_PIN       5       // Canal ADC1_4 (Seguro com Wi-Fi)
#define LED_IRRIGACAO 6       // Saída para o LED
#define BUZZER_PIN    7       // Saída para o Buzzer

// Pinos I2C dedicados conforme o esquemático da imagem
#define I2C_SDA       8       // Pino SDA indicado na imagem
#define I2C_SCL       9       // Pino SCL indicado na imagem

#define TEMP_LIMITE_ALTA 30.0
#define SOLO_CRITICO     300

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Estufa Agrícola ESP32-S3 (N16R8) ---");

  pinMode(LED_IRRIGACAO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_IRRIGACAO, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Inicializa a barramento I2C nos pinos 8 e 9
  Wire.begin(I2C_SDA, I2C_SCL);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Estufa ESP32-S3");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  dht.begin();
  delay(2000);
  lcd.clear();
}

void loop() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int leituraLDR = analogRead(LDR_PIN);
  int leituraSolo = analogRead(POT_PIN);

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler o sensor DHT!");
    lcd.setCursor(0, 0);
    lcd.print("Erro no DHT!    ");
    delay(2000);
    return;
  }

  int pctSolo = map(leituraSolo, 0, 4095, 0, 100);

  // Controle da Irrigação
  if (leituraSolo < SOLO_CRITICO) {
    digitalWrite(LED_IRRIGACAO, HIGH);
  } else {
    digitalWrite(LED_IRRIGACAO, LOW);
  }

  // Controle da Exaustão/Alarme
  if (temperatura > TEMP_LIMITE_ALTA) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Display LCD
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperatura, 1);
  lcd.print((char)223);
  lcd.print("C U:");
  lcd.print(umidade, 0);
  lcd.print("%   ");

  lcd.setCursor(0, 1);
  lcd.print("Solo:");
  lcd.print(pctSolo);
  lcd.print("% L:");
  lcd.print(map(leituraLDR, 0, 4095, 0, 100));
  lcd.print("%   ");

  // Monitor Serial
  Serial.printf("Temp: %.1f°C | Umid: %.1f%% | Solo: %d%% | Luz ADC: %d\n",
                temperatura, umidade, pctSolo, leituraLDR);

  delay(2000);
}