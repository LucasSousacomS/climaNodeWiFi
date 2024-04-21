#include "utils.h"

wifi rede;
mqtt comm;
climate clima;

void setup() {
  Serial.begin(9600);
  clima.begin();
  rede.begin();
  comm.begin();
}

void loop() {  
  //Iniciando o servidor
   if (!client.connected()) {
    rede.reconnect();
  }
    
  //Iniciando o timer
  timer.run();
}

