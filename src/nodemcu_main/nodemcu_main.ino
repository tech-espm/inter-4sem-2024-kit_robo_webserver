#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>
#include "Dados.h"

#define byte_inicial '_'
#define byte_final '!'

#define bytes_pacote_total (1 + 1 + 3 + 3 + 1)

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

Servo servo;

static const char html[] PROGMEM = "<!DOCTYPE html><html lang=\"pt-br\"><head><meta charset=\"UTF-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><title>Controle Remoto</title></head><body></body></html>\r\n";

void enviarPaginaInicial() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(html);
}

void processarMovimento() {
  // GET de exemplo:
  // http://IP/enviarMovimento?v=_1234567!
  
  String query = server.arg("v");
  if (query && query.length() == bytes_pacote_total && query[0] == byte_inicial && query[bytes_pacote_total - 1] == byte_final) {
    // Encaminha o pacote para o Arduino Uno
    Serial.print(query);
  }
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent("OK\n");
}

void realizarLeitura() {
  // GET de exemplo:
  // http://IP/realizarLeitura
  
  servo.write(135);
  delay(1000);
  servo.write(0);
  delay(1000);
  
  // @@@ Envia uma requisição POST para a API com o mock de uma leitura
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent("Leitura OK\n");
}

void setup() {
  servo.attach(D2);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // @@@ Envia uma requisição POST para a API com o IP
  
  if (MDNS.begin("esp8266")) { Serial.println("MDNS responder started"); }
  
  server.on("/", enviarPaginaInicial);
  server.on("/processarMovimento", processarMovimento);
  server.on("/realizarLeitura", realizarLeitura);
  server.onNotFound(enviarPaginaInicial);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  MDNS.update();
}
