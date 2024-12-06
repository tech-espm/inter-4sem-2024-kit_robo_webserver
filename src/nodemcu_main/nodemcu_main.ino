#include <ESP8266WiFi.h>
// A biblioteca HttpClient padrão do Arduino é essa aqui:
// https://www.arduino.cc/reference/en/libraries/httpclient/
// Mas, como estamos utilizando o ESP8266, vamos utilizar a biblioteca própria dele:
// https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "Dados.h"

#define byte_inicial '_'
#define byte_final '!'

#define bytes_pacote_total (1 + 1 + 3 + 3 + 1)

#define ip_destino "192.168.179.202"
#define porta_destino 5000
#define porta_destino_front "8080"

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

static const char html[] PROGMEM = "<!DOCTYPE html><html lang=\"pt-br\"><head><meta charset=\"UTF-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><title>Controle Remoto</title></head><body><script src=\"http://" ip_destino ":" porta_destino_front "/controle.js\"></script></body></html>\r\n";

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
  // http://IP/movimento?v=_1234567!
  
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
  // http://IP/leitura
  
  // Simula o pressionamento do botão, com os motores parados
  Serial.print("_1000000!");

  // Aguarda o processamento do servo motor
  delay(2500);
  
  // Envia uma requisição POST para a API com o mock de uma leitura
  WiFiClient wiFiClient;
  
  HTTPClient httpClient;
  
  // begin() não realiza nenhum tipo de comunicação via rede, apenas
  // prepara o objeto para ser utilizado depois. Existe uma forma mais
  // prática, mas consome mais memória:
  httpClient.begin(wiFiClient, ip_destino, porta_destino, "/grava_leituras", true);
  
  // Configura o tempo máximo de espera.
  httpClient.setTimeout(30000);
  
  // Configura o cabeçalho HTTP Connection: close,
  // para pedir para o servidor encerrar a conexão TCP ao final.
  // Por padrão HTTPClient envia o cabeçalho Connection: keep-alive,
  // e tenta reaproveitar a conexão TCP.
  httpClient.setReuse(false);
  
  // Vamos precisar enviar ao menos um cabeçalho, com o tipo do dado
  // que estamos enviando para o servidor. Estou definindo application/octet-stream
  // apenas para que o retorno do servidor fique diferente do outro exemplo :)
  // Poderíamos enviar qualquer outro tipo, como application/json, text/plain,
  // application/x-www-form-urlencoded etc.
  httpClient.addHeader("Content-Type", "application/json");
  
  Serial.println("Enviando requisicao POST HTTP...");
  // Aqui pode ser enviado tanto um array de bytes como uma String:
  //https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h#L192
  // Importante!!! Aqui nenhum tipo de conversão é realizada pela classe HTTPClient!
  // Ou seja, precisamos saber direitinho o formato/codificação dos dados
  // que estão sendo enviados para o servidor!
  
  String dados = "{\"umidade\":";
  dados += random(10, 91);
  dados += ",\"quadrante\":";
  dados += random(1, 5);
  dados += "}";
  int statusCode = httpClient.POST(dados);
  
  // Valores negativos não são códigos HTTP válidos, e indicam
  // um erro de comunicação, ou do hardware.
  if (statusCode > 0) {
    Serial.print("Status code: ");
    Serial.println(statusCode);
    
    // O cabeçalho Content-Length não precisa ser pedido
    // explicitamente, porque ele é sempre tratado.
    Serial.print("Content-Length: ");
    Serial.println(httpClient.getSize());
    
    // Para ver todas as contantes disponíveis:
    // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h#L64
    if (statusCode == HTTP_CODE_OK) {
      // Podemos pedir para o httpClient ler todo o conteúdo
      // na memória, para depois ser trabalhado, ou podemos
      // simplesmente pedir para o conteúdo ser redirecionado
      // para outro stream, como a porta serial.
      //String body = httpClient.getString();
      //Serial.println(body);
      httpClient.writeToStream(&Serial);
    }
  } else {
    Serial.print("Ocorreu um erro de comunicacao: ");
    Serial.println(httpClient.errorToString(statusCode));
  }
  
  httpClient.end();
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent("Leitura OK\n");
}

void setup() {
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
  
  // Envia uma requisição POST para a API com o IP
  WiFiClient wiFiClient;
  
  HTTPClient httpClient;
  
  // begin() não realiza nenhum tipo de comunicação via rede, apenas
  // prepara o objeto para ser utilizado depois. Existe uma forma mais
  // prática, mas consome mais memória:
  httpClient.begin(wiFiClient, ip_destino, porta_destino, "/grava_ip", true);
  
  // Configura o tempo máximo de espera.
  httpClient.setTimeout(30000);
  
  // Configura o cabeçalho HTTP Connection: close,
  // para pedir para o servidor encerrar a conexão TCP ao final.
  // Por padrão HTTPClient envia o cabeçalho Connection: keep-alive,
  // e tenta reaproveitar a conexão TCP.
  httpClient.setReuse(false);
  
  // Vamos precisar enviar ao menos um cabeçalho, com o tipo do dado
  // que estamos enviando para o servidor. Estou definindo application/octet-stream
  // apenas para que o retorno do servidor fique diferente do outro exemplo :)
  // Poderíamos enviar qualquer outro tipo, como application/json, text/plain,
  // application/x-www-form-urlencoded etc.
  httpClient.addHeader("Content-Type", "application/json");
  
  Serial.println("Enviando requisicao POST HTTP...");
  // Aqui pode ser enviado tanto um array de bytes como uma String:
  //https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h#L192
  // Importante!!! Aqui nenhum tipo de conversão é realizada pela classe HTTPClient!
  // Ou seja, precisamos saber direitinho o formato/codificação dos dados
  // que estão sendo enviados para o servidor!
  
  String dados = "{\"ip\":\"";
  dados += WiFi.localIP().toString();
  dados += "\"}";
  int statusCode = httpClient.POST(dados);
  
  // Valores negativos não são códigos HTTP válidos, e indicam
  // um erro de comunicação, ou do hardware.
  if (statusCode > 0) {
    Serial.print("Status code: ");
    Serial.println(statusCode);
    
    // O cabeçalho Content-Length não precisa ser pedido
    // explicitamente, porque ele é sempre tratado.
    Serial.print("Content-Length: ");
    Serial.println(httpClient.getSize());
    
    // Para ver todas as contantes disponíveis:
    // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h#L64
    if (statusCode == HTTP_CODE_OK) {
      // Podemos pedir para o httpClient ler todo o conteúdo
      // na memória, para depois ser trabalhado, ou podemos
      // simplesmente pedir para o conteúdo ser redirecionado
      // para outro stream, como a porta serial.
      //String body = httpClient.getString();
      //Serial.println(body);
      httpClient.writeToStream(&Serial);
    }
  } else {
    Serial.print("Ocorreu um erro de comunicacao: ");
    Serial.println(httpClient.errorToString(statusCode));
  }
  
  httpClient.end();
  
  server.on("/", enviarPaginaInicial);
  server.on("/movimento", processarMovimento);
  server.on("/leitura", realizarLeitura);
  server.onNotFound(enviarPaginaInicial);
  
  server.begin();
  Serial.println("HTTP server started");
  
  randomSeed(millis());
}

void loop() {
  server.handleClient();
}
