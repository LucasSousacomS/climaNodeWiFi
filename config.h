#define hall1 33
#define hall2 32
#define encoder 15
#define bucket 17
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
const char* ssid = "***REMOVED***";
const char* pass = "***REMOVED***";
//Passand o nome do servidor MQTT (o broker, que no caso é o HiveMQ)
const char* mqtt_server = "broker.hivemq.com";
const float mmPorPulso = 1;
//Criando uma instância de um cliente WiFi
WiFiClient espClient;
//Criando umas instância do PubSubClient (biblioteca usada para conectar ao mqtt)
PubSubClient client(espClient);

//Matriz com os valores lidos para cada ângulo em ambos os sensores
float sensor1A[] = {2.69,2.61,2.51,2.46,2.41,2.35,2.29,2.23,2.18,2.14,2.11,2.09,2.09,2.11,2.14,2.18,2.24,2.29,2.36,2.42,2.48,2.51,2.55};
float sensor2A[] = {2.32,2.89,2.40,2.58,2.68, 2.69,2.62,2.52,2.43,2.38,2.33,2.30,2.25,2.15,2.01,1.86,1.76,1.75,1.81,1.89,1.96,2.00,2.03};
//Matriz com os ângulos possíveis para leitura do sensor
int dir[] =        {0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 240, 255, 270, 285, 300, 315, 330, 345};

// Matrizes com os valores mínimos e máximos dos dois sensores de efeito Hall para cada direção (cada índice corresponde a uma direção em graus da matriz anterior).
float sensor1min[] = {2.67, 2.64, 2.59, 2.60, 2.53, 2.46, 2.39, 2.33, 2.25, 2.17, 2.14, 2.11, 2.10, 2.10, 2.15, 2.20, 2.27, 2.34, 2.42, 2.49, 2.56, 2.62, 2.69};
float sensor1max[] = {2.75, 2.75, 2.72, 2.72, 2.68, 2.62, 2.55, 2.47, 2.40, 2.32, 2.26, 2.22, 2.19, 2.19, 2.27, 2.34, 2.41, 2.49, 2.56, 2.63, 2.69, 2.73, 2.72};
float sensor2min[] = {2.31, 2.42, 2.61, 2.64, 2.80, 2.85, 2.76, 2.67, 2.59, 2.56, 2.54, 2.52, 2.42, 2.25, 1.59, 1.42, 1.43, 1.48, 1.66, 1.84, 1.99, 2.10, 2.30};
float sensor2max[] = {2.57, 2.71, 2.91, 2.95, 2.99, 2.99, 2.95, 2.87, 2.77, 2.70, 2.66, 2.64, 2.62, 2.53, 2.03, 1.71, 1.60, 1.76, 1.94, 2.09, 2.21, 2.27, 2.46};

Adafruit_BMP280 bmp; //I2C
//dht11 DHT11;
DHT dht(dht11pino, dhttipo);
BH1750 lightMeter;

// Declaração de variáveis globais para remoção de leituras incorretas pelo encoder
  volatile long cont = 0;
unsigned long tempo = 0;
volatile long tempoAntes = 0;

unsigned long chuvaCont;

// Função para contagem de detecções acusadas pelo sensor do tipo barreira
void IRAM_ATTR isr(){
  if(millis() - tempoAntes > 20){ // Função de debounce (conta uma detecção apenas se ela ocorrer após 20 ms da detecção anterior)
    cont++;
    tempoAntes = millis();
  }  
}

// Função para contagem de detecções acusadas pelo sensor do tipo barreira
void IRAM_ATTR contaChuva(){
  if(millis() - tempoAntes > 20){ // Função de debounce (conta uma detecção apenas se ela ocorrer após 20 ms da detecção anterior)
    chuvaCont++;
    tempoAntes = millis();
  }  
}

SimpleTimer timer;

//Criação de uma variável global para recebimento do ângulo lido no sensor de direção do vento
static int angulo = 0;