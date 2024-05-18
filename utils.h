#include "config.h"
class wifi{
  public:
  void begin(){
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
};

class mqtt{
  public:
  void begin(){
    //Passando a informação do servidor MQTT e da porta a ser usada para comunicação
    client.setServer(mqtt_server, 1883);
  }
};
class climate{
  public:
  // Função responsável por converter a quantidade de leituras feitas pelo encoder em uma velocidade em RPM
  static void velocidade(){
    float contagemFloat = cont;
    float velocidade = (((contagemFloat/20))/3)*60;
    direcao();
    // Essa função também é responsável por chamar a função "enviar"
    enviar(velocidade);
    cont = 0;
  }
  void begin(){

    pinMode(encoder, INPUT);
    pinMode(bucket, INPUT_PULLUP);
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
    // Criação de um interrupt no pino 35, responsável por detectar leituras no encoder (medição de velocidade) e chamar a função ISR na mudança de 0 para 1 (obs: independente da escolha de rising, falling ou change, por algum motivo, no ESP32 este código considerou o interrupt em todas as mudanças de estado do pino 35)
    attachInterrupt(encoder, isr, CHANGE);

    attachInterrupt(bucket, contaChuva, FALLING);
    // Criando um interrupt no software responsável por chamar a função "velocidade" a cada 3000ms (3 segundos)
    timer.setInterval (3000, velocidade);
  }
  
  //Função responsável por enviar os valores para a plataforma Blynk
  static void enviar(float vel){  
    float temperatura = bmp.readTemperature(); // Leitura da temperatura
    float pressao = (bmp.readPressure())/100; // Leitura da pressão e conversão para hPa
    float umidade = dht.readHumidity(); // Leitura da umidade relativa
    float luminosidade = lightMeter.readLightLevel();
    float chuva = chuvaCont*mmPorPulso;
    const float teste = 13.14;
    //Criando uma variável do tipoo "StaticJsonDocument<80>" chamada "doc"
    StaticJsonDocument<150> doc;
    //Criando uma variável char chamada "output" com 80 colunas
    char output[150];
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
    doc["d"] = angulo;
    doc["c"] = chuva;
    Serial.print("chuva");
    Serial.println(chuva);
    Serial.print("chuva2");
    Serial.println(chuvaCont);
    // Transforma a variável "doc" em um documento json serializado ("t": temperatura, "p": pressão, etc) substituindo as variáveis pelos valores contidos nelas e coloca a string gerada dentro da variável "output"
    serializeJson(doc, output);
    Serial.print("Output: ");
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
  // Função responsável por fazer a interpretação da direção do vento de acordo com as leituras do sensor de efeito Hall
  static void direcao(){
    Serial.println("Entrou na direçao");
    //Leitura dos dois sensores
    float sensor1 = (analogRead(hall1)*5.00)/4095.00;
    float sensor2 = (analogRead(hall2)*5.00)/4095.00;
    //Filtragem dos valores dos dois sensores e conversão para um valor de tensão
  /*  float sensor1Filtrado = (filter1(sensor1)*5)/4095;
    float sensor2Filtrado = (filter2(sensor2)*5)/4095;*/
    Serial.println(sensor1);
    Serial.println(sensor2);

    //Utilizando as matrizes de valores máximos e mínimos de valores dos sensores de efeito Hall e, a partir delas, identificando o ângulo correto para o qual o sensor de direção está apontando
    for(int i = 0; i <22; i++){
      // O programa ficará apenas dentro da função "direcao" e "loop" até que a função "velocidade" seja chamada pelo interrupt a cada 3 segundos
      float sensorMatrix = (sensor1A[i]);
      if((sensor1 >= sensor1min[i] && sensor1 <=sensor1max[i]) && (sensor2 >= sensor2min[i] && sensor2 <= sensor2max[i])){
        angulo = dir[i];
        Serial.print ("Angulo = ");
        Serial.println(angulo);
        break;
      }
    }
  }
};