

# Estação Meteorológica de Baixo Custo usando ESP32 e Node-RED

Bem vindo ao ClimaNode WiFi! Este projeto consiste em uma estação meteorológica caseira, de baixo custo baseada em ESP32. Ao ser finalizada, será capaz de fazer leituras de pressão atmosférica, temperatura, umidade do ar, pluviômetro, direção e velocidade do vento. Todas as informações são encaminhadas para o NodeRED via MMQTT, para que possam ser mostradas em um dashboard em tempo real. Devido ao uso de MQTT, não há a ncessidade de conexão cabeada com a estação, permitindo que a mesma seja instalada em um local externo, desde que possua conexão a uma rede wi-fi.

## Índice

1. [Visão Geral do Projeto](#visão-geral-do-projeto)
2. [Componentes](#componentes)
3. [Instalação e Configuração](#instalação-e-configuração)
4. [Código](#código)
5. [Configuração do Dashboard no Node-RED](#configuração-do-dashboard-no-node-red)
6. [Configuração do MQTT](#configuração-do-mqtt)
7. [Uso](#uso)
8. [Solução de Problemas](#solução-de-problemas)
9. [Contribuições](#contribuições)

## Visão Geral do Projeto

Esta estação meteorológica coleta e processa dados de múltiplos sensores para monitorar temperatura, umidade, pressão, intensidade luminosa, direção do vento, velocidade do vento e precipitação. O microcontrolador ESP32 lê os dados desses sensores e os envia como um objeto JSON via MQTT para um dashboard no Node-RED para visualização.

## Componentes

- **ESP32**: Microcontrolador principal
- **BMP280**: Sensor de Temperatura e Pressão
- **DHT11**: Sensor de Umidade
- **BH1750**: Sensor de Intensidade Luminosa
- **Sensores de Efeito Hall (2)**: Para detecção da direção do vento
- **Sensor de Barreira Infravermelho**: Para medição da velocidade do vento (encoder)
- **Reed Switch**: Para medição de precipitação (pluviômetro de balde basculante)

## Diagrama de Ligações

Em breve!

## Instalação e Configuração

### Configuração de Hardware

1. **Conecte os Sensores**: Ligue os sensores aos pinos de IO do ESP32.
2. **Monte a Estação Meteorológica**: Certifique-se de que todos os sensores estejam bem fixados e posicionados corretamente para leituras precisas.

### Configuração de Software

1. **Configurar Arduino IDE**:
    - Instale a Arduino IDE a partir do [site oficial do Arduino](https://www.arduino.cc/en/software).
    - Adicione a placa ESP32 à Arduino IDE. Siga as instruções do [Guia de Instalação da Placa ESP32](https://github.com/espressif/arduino-esp32).
    - Instale as bibliotecas necessárias: `Adafruit BMP280`, `DHT sensor library`, `BH1750`, e `PubSubClient`.

2. **Clone este Repositório**:
    ```sh
    git clone https://github.com/LucasSousacomS/climaNodeWiFi.git
    cd climaNodeWiFi
    ```

3. **Carregar o Código no ESP32**:
    - Crie um arquivo chamado `secrets.h` e o abra na Arduino IDE.
    - Configure seu SSID e senha de Wi-Fi no código.
    - Configure os detalhes do seu broker MQTT no código.
    - Carregue o código no ESP32.

## Configuração do Dashboard no Node-RED

1. **Instale o Node-RED**: Siga as instruções no [site oficial do Node-RED](https://nodered.org/docs/getting-started/).
2. **Configure o Broker MQTT**: Você pode usar um broker MQTT local como o Mosquitto ou um broker em nuvem (como HiveMQ).
3. **Crie o Dashboard**:
    - Importe o arquivo `node-red-dashboard.json` fornecido neste repositório para sua instância do Node-RED.
    - Ajuste os nós MQTT para apontarem para seu broker MQTT.

## Configuração do MQTT

Certifique-se de que o ESP32 e o Node-RED estejam conectados ao mesmo broker MQTT. Configure as definições do broker MQTT tanto no código do ESP32 quanto no fluxo do Node-RED para corresponderem à sua configuração.

## Uso

Uma vez que tudo esteja configurado:
- Ligue o ESP32.
- O ESP32 começará a ler os dados dos sensores e publicá-los no broker MQTT.
- Abra o dashboard do Node-RED para visualizar os dados.

## Solução de Problemas

- **Sem Dados no Dashboard**:
    - Certifique-se de que o ESP32 está conectado ao Wi-Fi.
    - Verifique a configuração do broker MQTT.
    - Cheque as conexões e ligações dos sensores.
- **Leituras de Sensores Incorretas**:
    - Calibre os sensores, se necessário.
    - Certifique-se de que os sensores estão posicionados corretamente.

## Contribuições

Contribuições são bem-vindas! Por favor, faça um fork deste repositório e envie pull requests.


---


