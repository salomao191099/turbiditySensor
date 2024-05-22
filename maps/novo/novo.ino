#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include  <WiFiClientSecureBearSSL.h>
#include <OneWire.h>  
#include <DallasTemperature.h>
#define dados1 0

//Termopar
OneWire oneWire1(dados1);
DallasTemperature sensors1(&oneWire1);
String tmpc;

//Turbidez
// Define o pino de Leitura do Sensor
int SensorTurbidez = A0;
 
// Inicia as variáveis
int i;
float tensao;
float NTU;

String valor;
WiFiClient wifiClient;


// Definições de SSID e senha padrão
const char* DEFAULT_SSID = "iPhone Rommel";
const char* DEFAULT_PASSWORD = "acesso1111";

// Variáveis de armazenamento para SSID e senha fornecidos pelo usuário
String new_ssid;
String new_password;

// Timer para rastrear o tempo limite de 120 segundos
unsigned long start_time;
const unsigned long TIMEOUT = 120000;

// Definições da rede local
const char* AP_SSID = "ESP8266_Config";
const char* AP_PASSWORD = "12345678";

// Definições do DNS
const byte DNS_PORT = 53;
DNSServer dnsServer;
const char* DNS_NAME = "config";

// Instância do servidor web
AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);
  sensors1.begin();

  // Inicializa o ponto de acesso (AP)
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("Access Point Created");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Inicia o servidor DNS
  dnsServer.start(DNS_PORT, DNS_NAME, WiFi.softAPIP());

  // Define a rota para a página de configuração
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", 
      "<h1>ESP8266 Wi-Fi Configuration</h1>"
      "<form action=\"/get\" method=\"post\">"
      "SSID: <input type=\"text\" name=\"ssid\"><br>"
      "Password: <input type=\"text\" name=\"password\"><br>"
      "<input type=\"submit\" value=\"Submit\">"
      "</form>"
      "<p>Connect to this network and access <a href=\"http://config/\">http://config/</a> to configure Wi-Fi settings.</p>");
  });

  // Define a rota para receber os dados do formulário
  server.on("/get", HTTP_POST, [](AsyncWebServerRequest *request){
    int params = request->params();
    for(int i=0; i<params; i++){
      AsyncWebParameter* p = request->getParam(i);
      if (p->name() == "ssid") {
        new_ssid = p->value().c_str();
      } else if (p->name() == "password") {
        new_password = p->value().c_str();
      }
    }
    request->send(200, "text/html", "Received. ESP will now attempt to connect to the provided network.");
  });

  // Inicia o servidor
  server.begin();
  Serial.println("Server started");

  // Marca o tempo de início
  start_time = millis();
}

void loop() {
  // Processa solicitações DNS
  dnsServer.processNextRequest();

  // Verifica se o tempo limite foi atingido ou se as novas credenciais foram fornecidas
  if ((millis() - start_time > TIMEOUT) || (!new_ssid.isEmpty() && !new_password.isEmpty())) {
    // Desliga o AP
    WiFi.softAPdisconnect(true);
    delay(1000); // Espera um pouco para garantir que o AP foi desligado

    // Define as credenciais de conexão WiFi
    const char* ssid_to_connect = new_ssid.isEmpty() ? DEFAULT_SSID : new_ssid.c_str();
    const char* password_to_connect = new_password.isEmpty() ? DEFAULT_PASSWORD : new_password.c_str();

    // Conecta ao WiFi
    WiFi.begin(ssid_to_connect, password_to_connect);
    Serial.print("Connecting to ");
    Serial.println(ssid_to_connect);

    // Espera a conexão
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Para o loop principal para evitar reconexões
    while (true) {
      // Pode adicionar código adicional aqui para executar tarefas após a conexão WiFi
      tmpc = String(termopar(),2);
      turbidez() ;
      // Enviando dados via método GET - Inicio
      float dado = NTU;
      valor = String(dado,3); // Enviando dados
      std :: unique_ptr < BearSSL :: WiFiClientSecure > client ( new  BearSSL :: WiFiClientSecure );
      client -> setInsecure (); // HTTPS
      HTTPClient https;
      String URL = "https://dados.pagekite.me/receber-dados?nome=" + valor + tmpc;
      https.begin(* client, URL);
      // Fazendo a requisição GET
      int httpsCode = https.GET();
      if (httpsCode > 0) {
        String payload = https.getString();
        //Serial.println(payload); // Print response
        Serial.println(tmpc);
        Serial.println(URL); // Print URL
      }
      https.end(); // Finalizando requisição
      // Enviando dados via método GET - Fim
      delay(10000);
    }
  }
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

float termopar(){
  sensors1.requestTemperatures();
  return sensors1.getTempCByIndex(0);
}