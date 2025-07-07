/**
 * @file Config.h
 * @brief Configurações centralizadas para o projeto de comunicação ESP-NOW
 * @version 1.1
 * @date Julho/2025
 *
 * Este arquivo define constantes de configuração para rede,
 * hardware, temporização e comunicação ESP-NOW.
 */

#pragma once // Diretiva moderna para include guard

#include <cstdint> // Para tipos integrais de largura específica

/**
 * @namespace Config
 * @brief Namespace principal para configurações do sistema
 *
 * Organiza configurações em namespaces específicos para
 * melhor organização e clareza do código.
 */
namespace Config
{

  /**
   * @namespace Hardware
   * @brief Configurações de hardware específicas
   *
   * Contém definições de pinos e configurações de hardware.
   */
  namespace Hardware
  {
    /// @brief Taxa de transmissão serial
    /// @details Velocidade de comunicação para depuração e monitoramento
    constexpr uint32_t BAUD_RATE = 115200U;

    constexpr uint32_t GPS_RX = 16;
    constexpr uint32_t GPS_TX = 17;

    constexpr uint32_t ADC_PIN = 32;      // Pino ADC para leitura de tensão
    constexpr float ADC_MULTIPLIER = 2.0; // Porque estamos usando 10k e 10k
  }

  /**
   * @namespace Timing
   * @brief Configurações de temporização
   *
   * Gerencia intervalos de tempo para diferentes processos.
   */
  namespace Timing
  {
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
  namespace EspNow
  {
    /// @brief Canal de comunicação ESP-NOW
    /// @details Canal específico para transmissão de dados ESP-NOW
    constexpr uint8_t CHANNEL = 1;
    constexpr uint8_t broadcastAddress[] = {0x2B, 0xBC, 0xBB, 0x4B, 0xE4, 0xBD}; // Endereço do receptor
  }
}
