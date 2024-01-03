#define hall1 18
#define hall2 19
#define encoder 15
#define dht11pino 4
#define dhttipo DHT11

#include <WiFi.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp> 
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <SimpleTimer.h>
#include <BH1750.h>

//Definindo constantes com o nome da rede e senha do Wifi ao qual o ESP32 irá se conectar
const char* ssid = "";
const char* pass = "";
//Passand o nome do servidor MQTT (o broker, que no casoo é o HiveMQ)
const char* mqtt_server = "broker.hivemq.com";
//Criando uma instância de um cliente WiFi
WiFiClient espClient;
//Criando umas instância do PubSubClient (biblioteca usada para conectar ao mqtt)
PubSubClient client(espClient);


//Matriz com os valores lidos para cada ângulo em ambos os sensores
float sensor1A[] = {2.56,2.54,2.51,2.46,2.41,2.35,2.29,2.23,2.18,2.14,2.11,2.09,2.09,2.11,2.14,2.18,2.24,2.29,2.36,2.42,2.48,2.51,2.55};
float sensor2A[] = {2.13,2.25,2.40,2.58,2.68, 2.69,2.62,2.52,2.43,2.38,2.33,2.30,2.25,2.15,2.01,1.86,1.76,1.75,1.81,1.89,1.96,2.00,2.03};
//Matriz com os ângulos possíveis para leitura do sensor
int dir[] =        {0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 240, 255, 270, 285, 300, 315, 330, 345};

// Matrizes com os valores mínimos e máximos dos dois sensores de efeito Hall para cada direção (cada índice corresponde a uma direção em graus da matriz anterior).
float sensor1min[] = {2.55, 2.53, 2.49, 2.43, 2.38, 2.32, 2.26, 2.21, 2.16, 2.13, 2.10, 2.09, 2.09, 2.11, 2.12, 2.15, 2.20, 2.26, 2.31, 2.35, 2.39, 2.45, 2.50};
float sensor1max[] = {2.56, 2.55, 2.52, 2.48, 2.43, 2.38, 2.32, 2.26, 2.20, 2.14, 2.11, 2.12, 2.10, 2.10, 2.14, 2.18, 2.24, 2.29, 2.36, 2.42, 2.48, 2.51, 2.55};
float sensor2min[] = {2.08, 2.19, 2.32, 2.49, 2.63, 2.68, 2.62, 2.52, 2.43, 2.38, 2.33, 2.30, 2.25, 2.15, 2.01, 1.86, 1.76, 1.75, 1.75, 1.81, 1.89, 1.96, 2.00};
float sensor2max[] = {2.19, 2.32, 2.49, 2.63, 2.68, 2.69, 2.65, 2.57, 2.47, 2.41, 2.35, 2.31, 2.28, 2.20, 2.08, 1.93, 1.81, 1.76, 1.81, 1.89, 1.96, 2.00, 2.03};

Adafruit_BMP280 bmp; //I2C
//dht11 DHT11;
DHT dht(dht11pino, dhttipo);
BH1750 lightMeter;

// Declaração de variáveis globais para remoção de leituras incorretas pelo encoder
volatile long cont = 0;
unsigned long tempo = 0;
volatile long tempoAntes = 0;

// Função para contagem de detecções acusadas pelo sensor do tipo barreira
void IRAM_ATTR isr(){
  if(millis() - tempoAntes > 20){ // Função de debounce (conta uma detecção apenas se ela ocorrer após 20 ms da detecção anterior)
    cont++;
    tempoAntes = millis();
  }  
}

SimpleTimer timer;

//Criação de uma variável global para recebimento do ângulo lido no sensor de direção do vento
static int angulo = 0;

void setup() {
  Serial.begin(9600);
  pinMode(encoder, INPUT);
  // Iniciando todas as instâncias de bibliotecas necessárias
  Wire.begin();
  dht.begin();
  lightMeter.begin();
  //lightMeter.begin();

  if (!bmp.begin(0x76)) { /*Definindo o endereço I2C como 0x76. Mudar, se necessário, para (0x77)*/
    
    //Imprime mensagem de erro no caso de endereço invalido ou não localizado. Modifique o valor 
    Serial.println(F(" Não foi possível encontrar um sensor BMP280 válido, verifique a fiação ou tente outro endereço!"));
    while (1) delay(10);
  }
  
  //Conectando o ESP32 ao WiFi usandoo o SSID e a senha fornecidos nas constantes anteriores
  WiFi.begin(ssid, pass);

  //int contador = 0;

//Enquanto o status do WiFi for diferente de conectado, ele vai ficar em um loop printando vários pontos finais
 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

//Quando conectar, irá informar que conectou e mostrar o IP
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //Passando a informação do servidor MQTT e da porta a ser usada para comunicação
  client.setServer(mqtt_server, 1883);

  // Criação de um interrupt no pino 35, responsável por detectar leituras no encoder (medição de velocidade) e chamar a função ISR na mudança de 0 para 1 (obs: independente da escolha de rising, falling ou change, por algum motivo, no ESP32 este código considerou o interrupt em todas as mudanças de estado do pino 35)
  attachInterrupt(encoder, isr, CHANGE);
  // Criando um interrupt no software responsável por chamar a função "velocidade" a cada 3000ms (3 segundos)
  timer.setInterval (3000, velocidade);
  
}

// Função responsável por fazer a interpretação da direção do vento de acordo com as leituras do sensor de efeito Hall
void direcao(){
  //Leitura dos dois sensores
  float sensor1 = ((analogRead(hall1))*5)/4095;
  float sensor2 = ((analogRead(hall2))*5)/4095;
  //Filtragem dos valores dos dois sensores e conversão para um valor de tensão
/*  float sensor1Filtrado = (filter1(sensor1)*5)/4095;
  float sensor2Filtrado = (filter2(sensor2)*5)/4095;*/

  //Utilizando as matrizes de valores máximos e mínimos de valores dos sensores de efeito Hall e, a partir delas, identificando o ângulo correto para o qual o sensor de direção está apontando
  for(int i = 0; i <22; i++){
    // O programa ficará apenas dentro da função "direcao" e "loop" até que a função "velocidade" seja chamada pelo interrupt a cada 3 segundos
    float sensorMatrix = (sensor1A[i]);
    if((sensor1 >= sensor1min[i] && sensor1 <=sensor1max[i]) && (sensor2 >= sensor2min[i] && sensor2 <= sensor2max[i])){
      angulo = dir[i];
      break;
    }
  }
}

// Função responsável por converter a quantidade de leituras feitas pelo encoder em uma velocidade em RPM
void velocidade(){
  float contagemFloat = cont;
  float velocidade = (((contagemFloat/20))/3)*60;
  // Essa função também é responsável por chamar a função "enviar"
  enviar(velocidade);
  cont = 0;
}

//Função responsável por enviar os valores para a plataforma Blynk
void enviar(float vel){  
  float temperatura = bmp.readTemperature(); // Leitura da temperatura
  float pressao = (bmp.readPressure())/100; // Leitura da pressão e conversão para hPa
  float umidade = dht.readHumidity(); // Leitura da umidade relativa
  float luminosidade = lightMeter.readLightLevel();
  //Criando uma variável do tipoo "StaticJsonDocument<80>" chamada "doc"
  StaticJsonDocument<80> doc;
  //Criando uma variável char chamada "output" com 80 colunas
  char output[80];
  /*client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close"); 
  client.println(" ");
  client.println("{");
  client.println("{ 'type': 'temperatura','value':");
  client.println(temperatura);
  client.println("},");
  client.println("{ 'type': 'pressao','value':");
  client.println(pressao);
  client.println("},");
  client.println("{ 'type': 'umidade','value':");
  client.println(umidade);
  client.println("},");
  client.println("{ 'type': 'luminosidade','value':");
  client.println(luminosidade);
  client.println("},");
  client.println("{ 'type': 'velocidade','value':");
  client.println(vel);
  client.println("},");*/
  //Criando diversos tipos e ditando seus valores, para montagem do doocumento JSON ("t", por exemplo é o tipo e "temperatura é o valor ligado ao tipo "t")
  doc["t"] = temperatura;
  doc["p"] = pressao;
  doc["u"] = umidade;
  doc["l"] = luminosidade;
  doc["v"] = vel;
  // Transforma a variável "doc" em um documento json serializado ("t": temperatura, "p": pressão, etc) substituindo as variáveis pelos valores contidos nelas e coloca a string gerada dentro da variável "output"
  serializeJson(doc, output);
  Serial.println(output);
  //Publica a string em "output" no /casa/clima, que é o endereço do mqtt conectado no NodeRED. Quando o NodeRED receber esse arquivo, ele vai desmontar o arquivo JSON e separar t, p, u, l e v e mostrar todos eles em gráficos
  client.publish("/casa/clima", output);
  /*Serial.print(temperatura);
  Serial.print(",");
  Serial.print(pressao);
  Serial.print(",");
  Serial.print(umidade);
  Serial.print(",");
  Serial.print(luminosidade);
  Serial.print(",");
  Serial.println(vel);*/
}

void loop() {  
  //Iniciando o servidor
   if (!client.connected()) {
    reconnect();
  }
    
  //Iniciando o timer
  timer.run();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}