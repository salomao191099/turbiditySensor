#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include  <WiFiClientSecureBearSSL.h>

//Variáveis
String valor;

const char* ssid = "HDPS";
const char* password = "Salomao112290";

WiFiClient wifiClient;
void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a rede Wifi ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  valor = "DADO"; // Enviando dados
  std :: unique_ptr < BearSSL :: WiFiClientSecure > client ( new  BearSSL :: WiFiClientSecure );
  client -> setInsecure (); // HTTPS
  HTTPClient https;
  String URL = "https://5f37cd0f-e92c-413c-86cb-a656a4476217-00-1h7grimy3xc0g.janeway.replit.dev/receber-dados?nome=" + valor;
  https.begin(* client, URL);
  // Fazendo a requisição GET
  int httpsCode = https.GET();
  if (httpsCode > 0) {
    String payload = https.getString();
    Serial.println(payload); // Print response
    Serial.println(URL); // Print URL
  }
  https.end(); // Finalizando requisição
  // Enviando dados via método GET - Fim
  delay(10000);
}