<!-- # MONITORAMENTO-ESTUFA

INTEGRANTES: Pedro Peruzzo, João Guilherme, Felipe Galdino, Sebastian Figueroa, Luiz Muraski.

 A estufa tem sensores que marcam a umidade, o nível de água, luminosidade, e temperatura. Além de possuir alertas visuais para falta de água, e sonoros para temperatura elevada. O objetivo desse projeto é integrar esses sensores a um site e aplicativos, que mostrarão todos esses dados.

COMPONENTES UTILIZADOS: 
1x Placa ESP32
1x Protoboard
1x Display LCD 16×2 I2C
1x Sensor DHT22/DHT11
1x Módulo LDR
1x Potenciômetro 10 kΩ
1x LED 5 mm (Irrigação)
1x Buzzer 5 V
1x Resistor 330
1x Resistor 10k

ESQUEMA ELÉTRICO:
GPIO 4 - LDR
GPIO 5 - POTENCIÔMETRO
GPIO 6 - LED
GPIO 7 - BUZZER
GPIO 15 - DHT
GPIO 8 - SDA
GPIO 9 - SCL

BIBLIOTECAS UTILIZADAS:
Arduino.h
WiFi.h
PubSubClient.h
ArduinoJson.h
Wire.h
LiquidCrystal_I2C.h
DHT.h

ACESSO AO DASHBOARD:
https://leomzito.github.io/Projeto_Estufa_Monitoramento/app.html
 -->