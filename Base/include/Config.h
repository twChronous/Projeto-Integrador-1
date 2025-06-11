/**
 * @file Config.h
 * @brief Configurações centralizadas para o projeto de comunicação ESP-NOW
 * @version 1.0
 * @date Maio/2025
 * 
 * Este arquivo define constantes de configuração para rede, 
 * hardware, temporização e comunicação ESP-NOW.
 */

 #pragma once  // Diretiva moderna para include guard

 #include <cstdint>       // Para tipos integrais de largura específica
 
 /**
  * @namespace Config
  * @brief Namespace principal para configurações do sistema
  * 
  * Organiza configurações em namespaces específicos para 
  * melhor organização e clareza do código.
  */
 namespace Config {
     /**
      * @namespace Network
      * @brief Configurações de rede e comunicação
      * 
      * Contém constantes para configuração de WiFi, 
      * endereçamento e comunicação de rede.
      */
     namespace Network {
         /// @brief Nome do Access Point WiFi
         /// @details SSID utilizado para identificação da rede
         constexpr const char* SSID = "ESP-NOW-Server";        
 
         /// @brief Senha do Access Point WiFi
         /// @details Chave de segurança para conexão na rede
         constexpr const char* PASSWORD = "12345678";          
 
         /// @brief Canal WiFi para comunicação
         /// @details Canal utilizado para transmissão de dados
         constexpr uint8_t WIFI_CHANNEL = 1U;                 
 
         /// @brief Porta padrão para servidor HTTP
         /// @details Porta utilizada para serviço web
         constexpr uint16_t HTTP_PORT = 80U;                  
         
         /// @brief Endereço IP do Access Point
         /// @details Endereço IP fixo para o ponto de acesso
         constexpr const char* AP_IP = "192.168.4.1";
         
         /// @brief Máscara de sub-rede
         /// @details Configura o range de endereços da rede local
         constexpr const char* SUBNET_MASK = "255.255.255.0";
         
         /// @brief Endereço MAC para comunicação broadcast
         /// @details Endereço utilizado para transmissão para todos os dispositivos
         /// @note Substitua pelo endereço MAC real do seu dispositivo
         constexpr uint8_t BROADCAST_ADDRESS[6] = {0x10, 0x06, 0x1C, 0x69, 0xC1, 0x44};
     }
 
     /**
      * @namespace Hardware
      * @brief Configurações de hardware específicas
      * 
      * Contém definições de pinos e configurações de hardware.
      */
     namespace Hardware {
         /// @brief Pino do LED interno
         /// @details Número do pino GPIO para controle do LED
         constexpr uint8_t LED_PIN = 2U;               
 
         /// @brief Taxa de transmissão serial
         /// @details Velocidade de comunicação para depuração e monitoramento
         constexpr uint32_t BAUD_RATE = 115200U;

         constexpr uint32_t ADC_PIN = 13;     // Pino ADC para leitura de tensão
         constexpr float ADC_MULTIPLIER = 2.0;  // Porque estamos usando 10k e 10k
     }
 
     /**
      * @namespace Timing
      * @brief Configurações de temporização
      * 
      * Gerencia intervalos de tempo para diferentes processos.
      */
     namespace Timing {
         /// @brief Tempo máximo de conexão WiFi
         /// @details Limite de tempo para estabelecer conexão WiFi (em milissegundos)
         constexpr uint16_t WIFI_CONNECT_TIMEOUT = 10000U;  
 
         /// @brief Intervalo de atualização da página web
         /// @details Período entre atualizações da interface web (em milissegundos)
         constexpr uint16_t WEB_REFRESH_INTERVAL = 2000U;   
     }
 
     /**
      * @namespace EspNow
      * @brief Configurações específicas para protocolo ESP-NOW
      * 
      * Gerencia parâmetros de comunicação ESP-NOW.
      */
     namespace EspNow {
         /// @brief Canal de comunicação ESP-NOW
         /// @details Canal específico para transmissão de dados ESP-NOW
         constexpr uint8_t CHANNEL = 1;
         constexpr uint8_t broadcastAddress[] = {0x10, 0x06, 0x1C, 0x69, 0xC1, 0x44};
     }
 }