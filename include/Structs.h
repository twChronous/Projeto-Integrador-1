/**
 * @file SensorStructs.h
 * @brief Definições de estruturas de dados para sensores
 * @version 1.0
 * @date Maio/2025
 * 
 * Este arquivo define as estruturas de dados utilizadas para 
 * armazenar informações de sensores de movimento e altitude.
 */

 #ifndef SENSOR_STRUCTS_H
 #define SENSOR_STRUCTS_H
 
 #include <cstdint>  // Para tipos de inteiro de tamanho fixo
 
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
 struct AcelerometerData {
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
 struct AltimeterData {
     /// @brief Pressão atmosférica em hectopascais (hPa)
     /// @details Representa a pressão atmosférica no momento da leitura
     float pressure;  
 
     /// @brief Altitude calculada em metros
     /// @details Altitude derivada da medição de pressão atmosférica
     float altitude;  
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
 struct SensorData {
     /// @brief Dados do acelerômetro e giroscópio
     /// @details Contém informações de movimento e orientação
     AcelerometerData acelerometro;  
 
     /// @brief Dados do altímetro
     /// @details Contém informações de pressão e altitude
     AltimeterData altimetro;        
 
     /// @brief Carimbo de tempo da leitura
     /// @details Marca temporal da coleta dos dados dos sensores
     float timestamp;                
 };
 #pragma pack(pop)
 
 #endif // SENSOR_STRUCTS_H
 