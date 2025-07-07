/**
 * @file SensorStructs.h
 * @brief Definições de estruturas de dados para sensores
 * @version 1.3
 * @date Julho/2025
 *
 * Este arquivo define as estruturas de dados utilizadas para
 * armazenar informações de sensores de movimento e altitude.
 */

#ifndef SENSOR_STRUCTS_H
#define SENSOR_STRUCTS_H

#include <cstdint> // Para tipos de inteiro de tamanho fixo

/**
 * @brief Estrutura de dados do acelerômetro e giroscópio
 *
 * Armazena informações de aceleração, rotação, temperatura
 * e ângulos de orientação.
 *
 * @note Uso de #pragma pack para garantir alinhamento de bytes
 * consistente entre diferentes plataformas
 */
#pragma pack(push, 1)
struct AcelerometerData
{
  /// @brief Aceleração no eixo X em metros por segundo ao quadrado
  /// @details Representa a aceleração linear no eixo X
  float accX;

  /// @brief Aceleração no eixo Y em metros por segundo ao quadrado
  /// @details Representa a aceleração linear no eixo Y
  float accY;

  /// @brief Aceleração no eixo Z em metros por segundo ao quadrado
  /// @details Representa a aceleração linear no eixo Z
  float accZ;

  /// @brief Velocidade angular no eixo X em graus por segundo
  /// @details Representa a rotação no eixo X
  float gyroX;

  /// @brief Velocidade angular no eixo Y em graus por segundo
  /// @details Representa a rotação no eixo Y
  float gyroY;

  /// @brief Velocidade angular no eixo Z em graus por segundo
  /// @details Representa a rotação no eixo Z
  float gyroZ;

  /// @brief Temperatura do sensor em graus Celsius
  /// @details Temperatura medida pelo sensor de movimento
  float temp;

  /// @brief Ângulo de arfagem (pitch) em graus
  /// @details Rotação ao redor do eixo Y
  float pitch;

  /// @brief Ângulo de rolagem (roll) em graus
  /// @details Rotação ao redor do eixo X
  float roll;
};
#pragma pack(pop)

/**
 * @brief Estrutura de dados do altímetro
 *
 * Armazena informações de pressão atmosférica e altitude calculada.
 *
 * @note Uso de #pragma pack para garantir alinhamento de bytes
 * consistente entre diferentes plataformas
 */
#pragma pack(push, 1)
struct AltimeterData
{
  /// @brief Pressão atmosférica em hectopascais (hPa)
  /// @details Representa a pressão atmosférica no momento da leitura
  float pressure;

  /// @brief Altitude calculada em metros
  /// @details Altitude derivada da medição de pressão atmosférica
  float altitude;
};
#pragma pack(pop)

/**
 * @brief Estrutura de dados do sensor de tensão
 *
 * Armazena informações de tensão elétrica da bateria.
 *
 * @note Uso de #pragma pack para garantir alinhamento de bytes
 * consistente entre diferentes plataformas
 */
#pragma pack(push, 1)
struct VoltageData
{
  /// @brief Tensão medida em volts (V)
  /// @details Representa a tensão elétrica no momento da leitura do foguete
  float voltage_rocket;
};
#pragma pack(pop)

/**
 * @brief Estrutura de dados do GPS
 *
 * Armazena informações de posicionamento geográfico.
 *
 * @note Uso de #pragma pack para garantir alinhamento de bytes
 * consistente entre diferentes plataformas
 */
#pragma pack(push, 1)
struct GPSData
{
  /// @brief Latitude em graus decimais
  /// @details Representa a posição geográfica no eixo norte-sul
  float latitude;

  /// @brief Longitude em graus decimais
  /// @details Representa a posição geográfica no eixo leste-oeste
  float longitude;

  /// @brief Altitude calculada em metros
  /// @details Altitude derivada da medição de posicionamento GPS
  float altitude;

  /// @brief Dia do mês segundo o GPS
  /// @details Representa o dia do mês da leitura GPS horario de Greenwich
  int day;
  /// @brief Mês do ano segundo o GPS
  /// @details Representa o mês do ano da leitura GPS horario de Greenwich
  int month;
  /// @brief Ano segundo o GPS
  /// @details Representa o ano da leitura GPS horario de Greenwich
  int year;
  /// @brief Hora do dia segundo o GPS
  /// @details Representa a hora do dia da leitura GPS horario de Greenwich
  int hour;
  /// @brief Minuto do dia segundo o GPS
  /// @details Representa o minuto do dia da leitura GPS horario de Greenwich
  int minute;
  /// @brief Segundo do dia segundo o GPS
  /// @details Representa o segundo do dia da leitura GPS horario de Greenwich
  int second;
};
#pragma pack(pop)
/**
 * @brief Estrutura consolidada de dados de sensores
 *
 * Combina dados do acelerômetro, altímetro e timestamp em
 * uma única estrutura para transmissão e processamento.
 *
 * @note Uso de #pragma pack para garantir alinhamento de bytes
 * consistente entre diferentes plataformas
 */
#pragma pack(push, 1)
struct SensorData
{
  /// @brief Dados do acelerômetro e giroscópio
  /// @details Contém informações de movimento e orientação
  AcelerometerData acelerometro;

  /// @brief Dados do altímetro
  /// @details Contém informações de pressão e altitude
  AltimeterData altimetro;

  /// @brief Dados do sensor de tensão
  /// @details Contém informações de tensão elétrica da bateria
  VoltageData tensao;

  /// @brief Dados do GPS
  /// @details Contém informações de posicionamento geográfico
  GPSData gps;

  /// @brief Carimbo de tempo da leitura
  /// @details Marca temporal da coleta dos dados dos sensores
  float timestamp;
};
#pragma pack(pop)

#endif // SENSOR_STRUCTS_H