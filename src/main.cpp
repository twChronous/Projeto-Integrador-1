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
 
 /// @brief Servidor web na porta 80
 WebServer server(80);
 
 /// @brief Flag para indicar atualização de dados
 volatile bool dadosAtualizados = false;
 
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
 void handleJSON() {
    String jsonResponse = "{\"sensors\":{" 
      "\"altimetro\":{" 
        "\"altitude\":" + String(dadosRecebidos.altimetro.altitude, 2) + 
        ",\"pressure\":" + String(dadosRecebidos.altimetro.pressure, 2) + 
      "}," 
      "\"acelerometro\":{" 
        "\"accX\":" + String(dadosRecebidos.acelerometro.accX, 2) + 
        ",\"accY\":" + String(dadosRecebidos.acelerometro.accY, 2) + 
        ",\"accZ\":" + String(dadosRecebidos.acelerometro.accZ, 2) + 
        ",\"gyroX\":" + String(dadosRecebidos.acelerometro.gyroX, 2) + 
        ",\"gyroY\":" + String(dadosRecebidos.acelerometro.gyroY, 2) + 
        ",\"gyroZ\":" + String(dadosRecebidos.acelerometro.gyroZ, 2) + 
        ",\"temp\":" + String(dadosRecebidos.acelerometro.temp, 2) + 
        ",\"roll\":" + String(dadosRecebidos.acelerometro.roll, 2) + 
        ",\"pitch\":" + String(dadosRecebidos.acelerometro.pitch, 2) + 
      "}" 
      ",\"timestamp\":" + String(dadosRecebidos.timestamp) + 
    "}}";
  
    server.send(200, "application/json", jsonResponse);
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
    
    // Configuração do canal WiFi e ESP-NOW
    configureEspNowChannel();
    
    // Inicialização do ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Erro ao iniciar ESP-NOW");
      ESP.restart();
      return;
    }
        // Configuração do callback de recebimento
    esp_now_register_recv_cb(onEspNowReceive);

    // Rotas do servidor web
    server.on("/", handleRoot);
    server.on("/json", handleJSON);

    // Inicia servidor web
    server.begin();
    Serial.println("Servidor Web iniciado!");

    // Log do canal configurado
    uint8_t currentChannel;
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
 
     // Pequeno delay para evitar travamentos
     delay(10);
 }
 