#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
// #include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#define IR_SEND_PIN 13
#define LED_BUILTIN 2
// #include <IRremote.hpp>

#include <IRremoteESP8266.h>
#include <IRsend.h>

const char* ssid = "rede";
const char* password = "senha";
IRsend irsend(IR_SEND_PIN); 

const int pinLed = 12;
const int relayPin = 14; 
const int motor1Pin = 27;

// Criação do servidor web
WebServer server(80);

uint16_t rawData[] = {8974, 4466,  564, 562,  564, 562,  564, 1676,  564, 562,  564, 562,  564, 562,  564, 562,  564, 562,  564, 1676,  564, 1676,  564, 560,  564, 1676,  564, 1674,  564, 1676,  564, 1674,  564, 1676,  564, 562,  564, 562,  566, 562,  564, 1676,  564, 562,  564, 562,  564, 562,  564, 560,  564, 1674,  566, 1674,  564, 1676,  564, 562,  564, 1676,  564, 1676,  564, 1676,  564, 1676,  564, 39700,  8950, 2240,  564};  // NEC 20DF10EF

// Função para ligar o LED
void handleLigar() {
  digitalWrite(pinLed, HIGH); // Liga o LED
  digitalWrite(relayPin, LOW);

  #if FLASHEND > 0x1FFF // For more than 8k flash => not for ATtiny85 etc.
    Serial.println(F("Send NEC 8 bit address=0x04 (0xFB04) and command 0x18 (0xE718) with exact timing (16 bit array format)"));
    Serial.flush();
    // IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
IrSender.sendNEC(0xF7, 0x08FB04, NEC_KHZ);
    delay(1000); // delay must be greater than 8 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal

  #endif

  server.send(200, "text/plain", "LED Ligado");
}

// Função para desligar o LED
void handleDesligar() {
  digitalWrite(relayPin, HIGH);
  digitalWrite(pinLed, LOW); // Desliga o LED
  server.send(200, "text/plain", "LED Desligado");
}

// Função para rota desconhecida
void handleNotFound() {
  server.send(404, "text/plain", "Rota não encontrada");
}

void handleLuz() {
  if (server.method() == HTTP_GET) {
    // Captura o valor da requisição GET
    String valor = server.arg("luz");  // Captura o parâmetro 'luz'
    
    // Verifica se o valor foi recebido corretamente
    Serial.println(valor);
    
    if (valor.equals("true")) {
      digitalWrite(relayPin, LOW); // Liga o rele (LED)
      server.send(200, "text/plain", "LUZ Ligada");
      Serial.println("LUZ Ligada");
    } else if (valor.equals("false")) {
      digitalWrite(relayPin, HIGH); // Desliga o rele (LUZ)
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
    // Captura o valor da requisição GET
    String valor = server.arg("motor");  // Captura o parâmetro 'luz'
    
    // Verifica se o valor foi recebido corretamente
    Serial.println(valor);
    
    if (valor.equals("true")) {
      digitalWrite(motor1Pin, HIGH); // Liga o rele (LED)
      server.send(200, "text/plain", "Motor Ligado");
      Serial.println("Motor Ligado");
    } else if (valor.equals("false")) {
      digitalWrite(motor1Pin, LOW); // Desliga o rele (LED)
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
      irsend.sendRaw(rawData, 67, 38);  // Send a raw data capture at 38kHz.
      server.send(200, "text/plain", "TV Ligada");
      Serial.println("TV Ligada");
    } else if (valor.equals("false")) {
      irsend.sendRaw(rawData, 67, 38);  // Send a raw data capture at 38kHz.
      server.send(200, "text/plain", "TV Desligado");
      Serial.println("TV Desligada");
    } else {
      server.send(400, "text/plain", "Valor inválido");
      Serial.println("Valor inválido");
    }
  }
}

void setup() {
  // Configuração do pino do LED
  pinMode(pinLed, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(motor1Pin, OUTPUT);
  digitalWrite(pinLed, LOW);
  digitalWrite(motor1Pin, HIGH);

  // Conexão com a rede Wi-Fi
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // IrSender.begin(); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
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
  server.on("/ligar", handleLigar);     // Rota para ligar o LED
  server.on("/desligar", handleDesligar); // Rota para desligar o LED
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
