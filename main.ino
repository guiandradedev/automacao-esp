#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DHT.h>
#include <ArduinoJson.h>

#include "config.h"

const int IRSendPin = 13;
const int pinLed = 12;
const int relayPin = 14; 
const int motor1Pin = 27;
const int tempPin = 27;

unsigned long previousMillis = 0;  // Armazena o último momento em que os dados foram enviados
const long interval = 1500;  // Intervalo de 1,5 segundos

IRsend irsend(IRSendPin); 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DHT dht(tempPin, DHT11);

uint16_t rawData[] = {8974, 4466,  564, 562,  564, 562,  564, 1676,  564, 562,  564, 562,  564, 562,  564, 562,  564, 562,  564, 1676,  564, 1676,  564, 560,  564, 1676,  564, 1674,  564, 1676,  564, 1674,  564, 1676,  564, 562,  564, 562,  566, 562,  564, 1676,  564, 562,  564, 562,  564, 562,  564, 560,  564, 1674,  566, 1674,  564, 1676,  564, 562,  564, 1676,  564, 1676,  564, 1676,  564, 1676,  564, 39700,  8950, 2240,  564};  // NEC 20DF10EF

void handleLuz(bool status);
void handleMotor1(bool status);
void handleTv(bool status);
void processMessage(String message);

void setup() {
  // Definição dos pinos
  pinMode(pinLed, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(motor1Pin, OUTPUT);

  irsend.begin();
  dht.begin();

  // Conexão com a rede Wi-Fi
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.begin();

  Serial.println("Servidor iniciado!");
}

void loop() {
  unsigned long currentMillis = millis();

  // Verifica se o intervalo passou
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Gera um valor aleatório entre 10 e 30
    // int v = random(10, 30);
    float temp = dht.readTemperature();
    float umidade = dht.readHumidity();

    // // Cria a mensagem com o valor gerado
    // String message = "Temperatura atual: " + String(temp) + "; Umidade: " + String(umidade);

    // // Verifica se o WebSocket está conectado
    // if (webSocket.isConnected()) {
    //   webSocket.sendTXT(message);
    //   // Serial.println("Mensagem enviada: " + message);
    // } else {
    //   Serial.println("WebSocket desconectado.");
    // }

    StaticJsonDocument<200> doc;
    doc["temp"] = temp;
    doc["humidity"] = umidade;
    doc["relay1"] = !digitalRead(relayPin);
    // doc["relay2"] = !digitalRead(relayPin);
    // doc["relay3"] = !digitalRead(relayPin);
    // doc["relay4"] = !digitalRead(relayPin);
    // doc["tv"] = digitalRead()

    // Converte para string
    String jsonString;
    serializeJson(doc, jsonString);

    // Envia via WebSocket
    ws.textAll(jsonString);
  }
}

void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Cliente %u conectado!\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("Cliente %u desconectado!\n", client->id());
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->opcode == WS_TEXT) {
      String message = "";
      for (size_t i = 0; i < len; i++) {
        message += (char)data[i];
      }
      Serial.printf("Mensagem recebida do cliente %u: %s\n", client->id(), message.c_str());

      // Processa o JSON recebido
      processMessage(message);
    }
  }
}

void handleLuz(bool status) {
  if(status) {
    digitalWrite(relayPin, LOW); 
    Serial.println("LUZ Ligada");
  } else {
    digitalWrite(relayPin, HIGH);
    Serial.println("LUZ Desligada");
  }
}

void handleMotor1(bool status) {
  if(status) {
      digitalWrite(motor1Pin, HIGH);
      Serial.println("Motor Ligado");
  } else {
      digitalWrite(motor1Pin, LOW); 
      Serial.println("Motor desligado");
  }
}

void handleTv(bool status) {
  irsend.sendRaw(rawData, 67, 38); 
  if(status) {
    Serial.println("TV Ligada");
  } else {
    Serial.println("TV Desligada");
  }
}


void processMessage(String message) {
  // Cria um objeto JSON
  StaticJsonDocument<200> doc;

  // Tenta analisar o JSON recebido
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("Erro ao analisar JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Extraia os valores do JSON
  String channel = doc["channel"];
  String content = doc["content"];

  Serial.println("Mensagem recebida:");
  Serial.print("Canal: ");
  Serial.println(channel);
  Serial.print("Conteudo: ");
  Serial.println(content);
  bool status = (content == "true");

  if (channel == "luz") {
    handleLuz(status);
  } else if (channel == "tv") {
    handleTv(status);
  } else if (channel == "motor1") {
    handleMotor1(status);
  } else if (channel == "ar") {
    // Ação para o canal "ar"
  } else if (channel == "msg") {
    // Ação para o canal "msg"
  } else {
    Serial.println("Canal desconhecido.");
  }
}

