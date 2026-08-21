<!-- # MONITORAMENTO-ESTUFA

INTEGRANTES: Pedro Peruzzo, João Guilherme, Felipe Galdino, Sebastian Figueroa, Luiz Muraski.

 A estufa tem sensores que marcam a umidade, o nível de água, luminosidade, entre outros. O objetivo desse projeto é integrar esses sensores a um site, que maostra todos esses dados, além de nos deixar controlar certas coisas dentro da estufa, como ligar e desligar o irrigador. O projeto facilita o manejo da estufa.

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