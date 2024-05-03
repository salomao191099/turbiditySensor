#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include  <WiFiClientSecureBearSSL.h>
 
// Define o pino de Leitura do Sensor
int SensorTurbidez = A0;
 
// Inicia as variáveis
int i;
float tensao;
float NTU;


const char* ssid = "HDPS";
const char* password = "Salomao112290";
String valor;
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
  turbidez() ;
  // Enviando dados via método GET - Inicio
  float dado = NTU;
  valor = String(dado,3); // Enviando dados
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

// Sistema de arredendamento para Leitura
float ArredondarPara( float ValorEntrada, int CasaDecimal ) {
  float multiplicador = powf( 10.0f, CasaDecimal );
  ValorEntrada = roundf( ValorEntrada * multiplicador ) / multiplicador;
  return ValorEntrada;
}

void turbidez(){
    // Inicia a leitura da tensao em 0
    tensao = 0;
    
    // Realiza a soma dos "i" valores de tensao
    for (i = 0; i < 800; i++) {
      tensao += ((float)analogRead(SensorTurbidez) / 1023) * 5;
    }
    
    // Realiza a média entre os valores lidos na função for acima
    tensao = tensao / 800;
    tensao = ArredondarPara(tensao, 1);
    
    // Se tensao menor que 2.5 fixa o valor de NTU
    if (tensao < 2.5) {
      NTU = 3000;
    }
    
    else if (tensao > 4.2) {
      NTU = 0;
      tensao = 4.2;
    }
    
    // Senão calcula o valor de NTU através da fórmula
    else {
      NTU = -1120.4 * tensao*tensao + 5742.3 * tensao - 4353.8;
    }
}