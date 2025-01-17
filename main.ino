#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "config.h"

const int IRSendPin = 13;
const int pinLed = 12;
const int relayPin = 14; 
const int motor1Pin = 27;

IRsend irsend(IRSendPin); 
WebServer server(80);

uint16_t rawData[] = {8974, 4466,  564, 562,  564, 562,  564, 1676,  564, 562,  564, 562,  564, 562,  564, 562,  564, 562,  564, 1676,  564, 1676,  564, 560,  564, 1676,  564, 1674,  564, 1676,  564, 1674,  564, 1676,  564, 562,  564, 562,  566, 562,  564, 1676,  564, 562,  564, 562,  564, 562,  564, 560,  564, 1674,  566, 1674,  564, 1676,  564, 562,  564, 1676,  564, 1676,  564, 1676,  564, 1676,  564, 39700,  8950, 2240,  564};  // NEC 20DF10EF

void handleNotFound() {
  server.send(404, "text/plain", "Rota não encontrada");
}

void handleLuz() {
  if (server.method() == HTTP_GET) {
    String valor = server.arg("luz"); 
    
    Serial.println(valor);
    
    if (valor.equals("true")) {
      digitalWrite(relayPin, LOW); 
      server.send(200, "text/plain", "LUZ Ligada");
      Serial.println("LUZ Ligada");
    } else if (valor.equals("false")) {
      digitalWrite(relayPin, HIGH);
      server.send(200, "text/plain", "LUZ Desligado");
      Serial.println("LUZ Desligada");
    } else {
      server.send(400, "text/plain", "Valor inválido");
      Serial.println("Valor inválido");
    }
  }
}
void handleMotor1() {
  if (server.method() == HTTP_GET) {
    String valor = server.arg("motor");
    
    Serial.println(valor);
    
    if (valor.equals("true")) {
      digitalWrite(motor1Pin, HIGH);
      server.send(200, "text/plain", "Motor Ligado");
      Serial.println("Motor Ligado");
    } else if (valor.equals("false")) {
      digitalWrite(motor1Pin, LOW); 
      server.send(200, "text/plain", "Motor desligado");
      Serial.println("Motor desligado");
    } else {
      server.send(400, "text/plain", "Valor inválido");
      Serial.println("Valor inválido");
    }
  }
}

void handleTv() {
  if (server.method() == HTTP_GET) {
    String valor = server.arg("tv"); 
      Serial.println(valor);
    
    if (valor.equals("true")) {
      irsend.sendRaw(rawData, 67, 38); 
      server.send(200, "text/plain", "TV Ligada");
      Serial.println("TV Ligada");
    } else if (valor.equals("false")) {
      irsend.sendRaw(rawData, 67, 38);
      server.send(200, "text/plain", "TV Desligado");
      Serial.println("TV Desligada");
    } else {
      server.send(400, "text/plain", "Valor inválido");
      Serial.println("Valor inválido");
    }
  }
}

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(motor1Pin, OUTPUT);

  // Conexão com a rede Wi-Fi
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  irsend.begin();

  Serial.println("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Configuração das rotas
  server.on("/luz", HTTP_GET, handleLuz);     // Rota para manipular a luz
  server.on("/tv", handleTv); // Rota para desligar o LED
  server.on("/motor1", handleMotor1); // Rota para desligar o LED
  server.onNotFound(handleNotFound);    // Rota padrão (404)

  // Inicializa o servidor
  server.begin();
  Serial.println("Servidor iniciado!");
  digitalWrite(relayPin, LOW);

  delay(3000);
  digitalWrite(relayPin, HIGH);

}

void loop() {
  server.handleClient(); // Processa requisições HTTP
}
