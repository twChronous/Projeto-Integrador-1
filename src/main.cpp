/**
 * @file main.cpp
 * @brief Receptor ESP-NOW com servidor web para monitoramento de sensores
 * @version 1.1
 * @date May/2025
 * @author Lucas Mateus
 */

 #include <Arduino.h>
 #include <WiFi.h>
 #include <esp_now.h>
 #include <WebServer.h>
 #include "Structs.h"
 #include <esp_wifi.h>
 #include "Config.h"
 
 /// @brief Dados globais recebidos via ESP-NOW
 SensorData dadosRecebidos = {0};


esp_now_peer_info_t peerInfo = {};

const float VREF = 3.3;  // Tensão de referência do ADC (ESP32 usa 3.3V)
 /// @brief Servidor web na porta 80
 WebServer server(80);
 
 /// @brief Flag para indicar atualização de dados
 volatile bool dadosAtualizados = false;
 
uint16_t nextSequenceId = 0;

 /** @brief Estrutura global para armazenamento de dados de telemetria */
 SensorData sensorData = {};

struct LaunchCommand {
  CommandType type;
  uint32_t timestamp;
  uint16_t sequenceId;
  uint8_t checksum;

  uint8_t calculateChecksum() const {
    // Simples checksum: soma de bytes
    uint8_t sum = static_cast<uint8_t>(type) + 
                  (timestamp & 0xFF) + ((timestamp >> 8) & 0xFF) + ((timestamp >> 16) & 0xFF) + ((timestamp >> 24) & 0xFF) +
                  (sequenceId & 0xFF) + ((sequenceId >> 8) & 0xFF);
    return sum;
  }
};
 /**
  * @brief Configura o canal e região do WiFi para ESP-NOW
  * 
  * Esta função define as configurações de país e canal para comunicação ESP-NOW,
  * garantindo compatibilidade regulatória e configuração correta do canal.
  */
 void configureEspNowChannel() {
     wifi_country_t country = {
         .cc = "BR",     // Código do país (Brasil)
         .schan = 1,     // Canal inicial
         .nchan = 13,    // Número de canais
         .policy = WIFI_COUNTRY_POLICY_AUTO
     };
 
     // Configura país e região
     esp_wifi_set_country(&country);
     
     // Define o canal específico
     wifi_second_chan_t secondChan = WIFI_SECOND_CHAN_NONE;
     esp_wifi_set_channel(Config::EspNow::CHANNEL, secondChan);
 }
 void onEspNowSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Falha no envio do comando ESP-NOW");
  }
}
 /**
  * @brief Gera página HTML com dados dos sensores
  * 
  * @return String contendo o HTML formatado com dados dos sensores
  * 
  * Cria uma página web responsiva com tabela de dados de sensores,
  * incluindo atualização automática a cada 2 segundos.
  */
 String formatarDadosHTML() {
    return "<!DOCTYPE html><html><head>"
       "<meta charset='utf-8'>"
       "<meta http-equiv='refresh' content='2'>"
       "<style>"
       "body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; }"
       "h1 { color: #333; }"
       "table { width: 100%; border-collapse: collapse; }"
       "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
       "th { background-color: #f2f2f2; }"
       "</style>"
       "<title>Dados ESP-NOW</title></head><body>"
       "<h1>Dados Recebidos via ESP-NOW</h1>"
       "<table>"
       "<tr><th>Sensor</th><th>Valor</th></tr>"
       "<tr><td>Canal ESP-NOW</td><td>" + String(Config::EspNow::CHANNEL) + "</td></tr>"
       "<tr><td>Acelerômetro X</td><td>" + String(dadosRecebidos.acelerometro.accX, 2) + "</td></tr>"
       "<tr><td>Acelerômetro Y</td><td>" + String(dadosRecebidos.acelerometro.accY, 2) + "</td></tr>"
       "<tr><td>Acelerômetro Z</td><td>" + String(dadosRecebidos.acelerometro.accZ, 2) + "</td></tr>"
       "<tr><td>Giroscópio X</td><td>" + String(dadosRecebidos.acelerometro.gyroX, 2) + "</td></tr>"
       "<tr><td>Giroscópio Y</td><td>" + String(dadosRecebidos.acelerometro.gyroY, 2) + "</td></tr>"
       "<tr><td>Giroscópio Z</td><td>" + String(dadosRecebidos.acelerometro.gyroZ, 2) + "</td></tr>"
       "<tr><td>Temp</td><td>" + String(dadosRecebidos.acelerometro.temp, 2) + "</td></tr>"
       "<tr><td>Roll</td><td>" + String(dadosRecebidos.acelerometro.roll, 2) + "</td></tr>"
       "<tr><td>Pitch</td><td>" + String(dadosRecebidos.acelerometro.pitch, 2) + "</td></tr>"
       "<tr><td>Altitude</td><td>" + String(dadosRecebidos.altimetro.altitude, 2) + "</td></tr>"
       "<tr><td>Pressure</td><td>" + String(dadosRecebidos.altimetro.pressure, 2) + "</td></tr>"
       "<tr><td>Voltage (Base)</td><td>" + String(dadosRecebidos.tensao.voltage_base, 2) + "</td></tr>"
       "<tr><td>Voltage (Rocket)</td><td>" + String(dadosRecebidos.tensao.voltage_rocket, 2) + "</td></tr>"
       "<tr><td>Latitude</td><td>" + String(dadosRecebidos.gps.latitude, 6) + "</td></tr>"
       "<tr><td>Longitude</td><td>" + String(dadosRecebidos.gps.longitude, 6) + "</td></tr>"
       "<tr><td>Altitude GPS</td><td>" + String(dadosRecebidos.gps.altitude, 2) + "</td></tr>"
       "<tr><td>Timestamp</td><td>" + String(dadosRecebidos.timestamp) + "</td></tr>"
       "</table>"
       "</body></html>";
 }
 
 /**
  * @brief Callback para recebimento de dados via ESP-NOW
  * 
  * @param info Informações sobre o remetente
  * @param incomingData Ponteiro para os dados recebidos
  * @param len Tamanho dos dados recebidos
  * 
  * Processa os dados recebidos, verifica integridade e atualiza 
  * variáveis globais com informações dos sensores.
  */
 void onEspNowReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    // Verifica o tamanho dos dados
    if (len != sizeof(SensorData)) {
      Serial.printf("Tamanho de dados inválido. Esperado: %d, Recebido: %d\n", 
                    sizeof(SensorData), len);
      return;
    }
    
    // Formata endereço MAC do remetente
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             info->src_addr[0], info->src_addr[1], info->src_addr[2],
             info->src_addr[3], info->src_addr[4], info->src_addr[5]);
    
    // Copia os dados recebidos
    memcpy(&dadosRecebidos, incomingData, sizeof(SensorData));
    
    // Marca dados como atualizados
    dadosAtualizados = true;
    
    // Log de recebimento
    Serial.println("Dados recebidos:");
    Serial.printf("MAC: %s\n", macStr);
    Serial.println("-----------");
    }
    
 
 /**
  * @brief Rota principal do servidor web
  * 
  * Renderiza a página HTML com os dados dos sensores
  */
 void handleRoot() {
     server.send(200, "text/html", formatarDadosHTML());
 }
 
 /**
  * @brief Gera resposta JSON com dados dos sensores
  * 
  * Cria uma estrutura JSON organizada com informações de altímetro, 
  * acelerômetro e timestamp.
  */
 // Retorna uma string JSON para os dados do altímetro. Ex: {"altitude":VAL,"pressure":VAL}
String getAltimetroPayloadJson() {
    return "{\"altitude\":" + String(dadosRecebidos.altimetro.altitude, 2) +
           ",\"pressure\":" + String(dadosRecebidos.altimetro.pressure, 2) + "}";
}

// Retorna uma string JSON para os dados do acelerômetro. Ex: {"accX":VAL,...,"pitch":VAL}
String getAcelerometroPayloadJson() {
    return "{\"accX\":" + String(dadosRecebidos.acelerometro.accX, 2) +
           ",\"accY\":" + String(dadosRecebidos.acelerometro.accY, 2) +
           ",\"accZ\":" + String(dadosRecebidos.acelerometro.accZ, 2) +
           ",\"gyroX\":" + String(dadosRecebidos.acelerometro.gyroX, 2) +
           ",\"gyroY\":" + String(dadosRecebidos.acelerometro.gyroY, 2) +
           ",\"gyroZ\":" + String(dadosRecebidos.acelerometro.gyroZ, 2) +
           ",\"temp\":" + String(dadosRecebidos.acelerometro.temp, 2) +
           ",\"roll\":" + String(dadosRecebidos.acelerometro.roll, 2) +
           ",\"pitch\":" + String(dadosRecebidos.acelerometro.pitch, 2) + "}";
}

// Retorna uma string JSON para os dados de tensão. Ex: {"voltage_base":VAL,"voltage_rocket":VAL}
String getTensaoPayloadJson() {
    return "{\"voltage_base\":" + String(sensorData.tensao.voltage_base, 2) +
           ",\"voltage_rocket\":" + String(dadosRecebidos.tensao.voltage_rocket, 2) + "}";
}

// Retorna uma string JSON para os dados do GPS. Ex: {"latitude":VAL,...,"second":VAL}
String getGpsPayloadJson() {
    return "{\"latitude\":" + String(dadosRecebidos.gps.latitude, 6) +
           ",\"longitude\":" + String(dadosRecebidos.gps.longitude, 6) +
           ",\"altitude\":" + String(dadosRecebidos.gps.altitude, 2) +
           ",\"day\":" + String(dadosRecebidos.gps.day) +
           ",\"month\":" + String(dadosRecebidos.gps.month) +
           ",\"year\":" + String(dadosRecebidos.gps.year) +
           ",\"hour\":" + String(dadosRecebidos.gps.hour) +
           ",\"minute\":" + String(dadosRecebidos.gps.minute) +
           ",\"second\":" + String(dadosRecebidos.gps.second) + "}";
}

String getBaseStationInfoJson() {
    return "\"esp_now_channel\":" + String(Config::EspNow::CHANNEL) +
           ",\"mac_address\":\"" + WiFi.macAddress() + "\"" + // MAC da interface STA
           ",\"timestamp\":" + String(dadosRecebidos.timestamp);
}
 void handleJSON() {
    String altimetroJson = getAltimetroPayloadJson();
    String acelerometroJson = getAcelerometroPayloadJson();
    String tensaoJson = getTensaoPayloadJson();
    String gpsJson = getGpsPayloadJson();
    String baseInfoJson = getBaseStationInfoJson();

    String jsonResponse = "{\"sensors\":{";
    jsonResponse += "\"altimetro\":" + altimetroJson + ",";
    jsonResponse += "\"acelerometro\":" + acelerometroJson + ",";
    jsonResponse += "\"tensao\":" + tensaoJson + ",";
    jsonResponse += "\"gps\":" + gpsJson + ","; // Vírgula aqui, pois baseInfoJson segue
    jsonResponse += baseInfoJson; // Esta já contém as chaves e não termina com vírgula
    jsonResponse += "}}";

    server.send(200, "application/json", jsonResponse);
}
// Handler para retornar apenas dados do altímetro
void handleAltimetroJSON() {
    String jsonResponse = "{\"altimetro\":" + getAltimetroPayloadJson() + "}";
    server.send(200, "application/json", jsonResponse);
}

// Handler para retornar apenas dados do acelerômetro
void handleAcelerometroJSON() {
    String jsonResponse = "{\"acelerometro\":" + getAcelerometroPayloadJson() + "}";
    server.send(200, "application/json", jsonResponse);
}

// Handler para retornar apenas dados de tensão
void handleTensaoJSON() {
    String jsonResponse = "{\"tensao\":" + getTensaoPayloadJson() + "}";
    server.send(200, "application/json", jsonResponse);
}

// Handler para retornar apenas dados do GPS
void handleGpsJSON() {
    String jsonResponse = "{\"gps\":" + getGpsPayloadJson() + "}";
    server.send(200, "application/json", jsonResponse);
}
void sendFlightCommand(CommandType type) {
  // Configuração de peer antes do envio
  memcpy(peerInfo.peer_addr, Config::EspNow::broadcastAddress, 6);
  peerInfo.channel = Config::EspNow::CHANNEL;
  peerInfo.encrypt = false;

  // Adiciona peer se ainda não existir
  if (esp_now_is_peer_exist(Config::EspNow::broadcastAddress) != true) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Falha ao adicionar peer");
      return;
    }
  }

  // Prepara o comando
  LaunchCommand cmd;
  cmd.type = type;
  cmd.timestamp = millis();
  cmd.sequenceId = nextSequenceId++;
  cmd.checksum = cmd.calculateChecksum();

  // Envia o comando
  esp_err_t result = esp_now_send(Config::EspNow::broadcastAddress, 
                                  reinterpret_cast<uint8_t*>(&cmd), 
                                  sizeof(cmd));
  
  if (result != ESP_OK) {
    Serial.printf("Falha ao enviar comando %s! Código de erro: %d\n", 
      type == CommandType::START_FLIGHT ? "INICIAR" : "ENCERRAR", result);
  }
}
 /**
  * @brief Função de configuração inicial do sistema
  * 
  * Inicializa comunicação serial, configura WiFi, ESP-NOW e 
  * servidor web para monitoramento de sensores.
  */
 void setup() {
    // Inicialização serial
    Serial.begin(115200);
    while(!Serial) { delay(10); }
    
    // Configuração do modo WiFi
    WiFi.mode(WIFI_AP_STA);  // Modo misto para ESP-NOW e AP
    WiFi.softAPConfig(Config::Network::AP_IP, Config::Network::AP_IP, Config::Network::SUBNET_MASK);
    WiFi.softAP(Config::Network::SSID, Config::Network::PASSWORD);
    
    // Log de configuração de rede
    Serial.println("Configurando Access Point");
    Serial.print("IP do servidor: ");
    Serial.println(WiFi.softAPIP());
    
    Serial.println("MAC da ESP32:");
    Serial.println(WiFi.softAPmacAddress());
    
    // Configuração do canal WiFi e ESP-NOW
    configureEspNowChannel();
    
    // Inicialização do ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Erro ao iniciar ESP-NOW");
      ESP.restart();
      return;
    }
    esp_now_register_recv_cb(onEspNowReceive);
    esp_now_register_send_cb(onEspNowSent);

    // Rotas do servidor web
    server.on("/", handleRoot);
    server.on("/json", handleJSON);
    server.on("/json/gps", handleGpsJSON);
    server.on("/json/tensao", handleTensaoJSON);
    server.on("/json/altimetro", handleAltimetroJSON);
    server.on("/json/acelerometro", handleAcelerometroJSON);

    server.onNotFound([]() {
        server.send(404, "text/plain", "404 Not Found");
    });
    server.on("/launch", []() {
        sendFlightCommand(CommandType::START_FLIGHT);
        server.send(200, "text/plain", "Comando de lançamento enviado!");
    });
    server.on("/arrival", []() {
        sendFlightCommand(CommandType::END_FLIGHT);
        server.send(200, "text/plain", "Comando de chegada enviado!");
    });

    // Inicia servidor web
    server.begin();
    Serial.println("Servidor Web iniciado!");

    // Log do canal configurado
    uint8_t currentChannel;
    esp_wifi_set_channel(Config::EspNow::CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_wifi_get_channel(&currentChannel, 0);
    Serial.printf("Canal ESP-NOW configurado: %d\n", currentChannel);
}
 /**
  * @brief Função de loop principal
  * 
  * Mantém o servidor web processando requisições e garante 
  * funcionamento contínuo do sistema.
  */
 void loop() {
     // Lida com requisições do servidor web
    server.handleClient();
    int leituraADC = analogRead(Config::Hardware::ADC_PIN);
    float tensaoPino = (leituraADC / 4095.0) * VREF;
    float tensaoReal = tensaoPino * Config::Hardware::ADC_MULTIPLIER;
    sensorData.tensao.voltage_base = tensaoReal;

     // Pequeno delay para evitar travamentos
     delay(100);
 }
 