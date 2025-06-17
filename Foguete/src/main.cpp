/**
 * @file main.cpp
 * @brief Sistema de Telemetria para Foguete d'Água
 * 
 * @details Este arquivo contém a implementação do sistema de telemetria 
 * para um foguete d'água, utilizando ESP32, MPU6050 e BMP280.
 * 
 * @author Lucas Mateus
 * @date Maio/2025
 * @version 1.1
 * 
 * @note Projeto Integrador 1 - Engenharia
 */
 #include "SD.h"
 #include "SPI.h"
 #include <Adafruit_MPU6050.h>
 #include <Adafruit_Sensor.h>
 #include <Wire.h>
 #include <esp_now.h>
 #include <WiFi.h>
 #include <esp_wifi.h>
 #include <Adafruit_BMP280.h>
 
 #include <Config.h>
 #include <Structs.h>
 
 // Constantes de configuração
 /** @brief Intervalo de leitura dos sensores (ms) */
 const unsigned long SENSOR_READ_INTERVAL = 100;   // 10 Hz
 
 /** @brief Intervalo de transmissão de dados (ms) */
 const unsigned long TRANSMISSION_INTERVAL = 500;  // 2 Hz
 
 /** @brief Coeficiente do filtro complementar para fusão sensorial */
 const float COMPLEMENTARY_FILTER_ALPHA = 0.98;


 // Constantes de estado de voo
enum FlightState {
  GROUND_STATE,
  PRE_FLIGHT,
  FLIGHT_ACTIVE,
  POST_FLIGHT
};

// Variáveis globais de estado
FlightState currentFlightState = GROUND_STATE;
String currentLogFilename = "";
bool sdReady = false;
bool isFlightLogging = false;
unsigned long flightStartTime;
const float VREF = 3.3;  // Tensão de referência do ADC (ESP32 usa 3.3V)

/** 
 * @brief Timestamp da última leitura de sensor
 * @details Utilizado para controlar a taxa de amostragem dos sensores
 * @see SENSOR_READ_INTERVAL
 */
unsigned long lastSensorReadTime = 0;

/** 
 * @brief Timestamp da última transmissão de dados
 * @details Controla a frequência de envio dos pacotes de telemetria
 * @see TRANSMISSION_INTERVAL
 */
unsigned long lastTransmissionTime = 0;

/** 
 * @brief Ângulos de inclinação calculados
 * 
 * @details Armazena os ângulos de pitch e roll obtidos 
 * através do filtro complementar
 * 
 * @var pitch Ângulo de arfagem (rotação no eixo Y)
 * @var roll Ângulo de rolamento (rotação no eixo X)
 * 
 * @note Valores em graus
 * @see updateSensorData()
 */
float pitch = 0.0, roll = 0.0;


 // Declaração de objetos globais
 /** @brief Objeto para comunicação com o sensor MPU6050 */
 Adafruit_MPU6050 mpu;
 
 /** @brief Objeto para comunicação com o sensor BMP280 */
 Adafruit_BMP280 bmp;
 
 /** @brief Estrutura global para armazenamento de dados de telemetria */
 SensorData sensorData = {};
 
 /** 
 * @brief Declarações de Funções do Sistema de Telemetria
 * @details Protótipos de funções para inicialização, 
 * coleta de dados, transmissão e depuração
 */

/**
 * @brief Inicializa todos os sensores do sistema
 * 
 * @details Configura e testa os sensores MPU6050 e BMP280
 * 
 * @return bool Indica se a inicialização foi bem-sucedida
 * @retval true Todos os sensores inicializados corretamente
 * @retval false Falha na inicialização de algum sensor
 * 
 * @note Sensores críticos: MPU6050 e BMP280
 * @warning Pode travar o sistema em caso de falha
 */
void setupSensors();

/**
 * @brief Atualiza os dados dos sensores
 * 
 * @details Realiza leitura dos sensores, aplica filtros 
 * e preenche a estrutura de dados de telemetria
 * 
 * @note Limita a taxa de amostragem para evitar sobrecarga
 * @see SENSOR_READ_INTERVAL
 * @see SensorData
 */
void updateSensorData();

/**
 * @brief Transmite dados de telemetria
 * 
 * @details Envia pacote de dados via protocolo ESP-NOW
 * para o endereço de broadcast
 * 
 * @note Limita a taxa de transmissão
 * @see TRANSMISSION_INTERVAL
 * @see Config::EspNow::broadcastAddress
 */
void transmitData();

/**
 * @brief Imprime dados de telemetria para depuração
 * 
 * @details Exibe informações detalhadas dos sensores 
 * no console serial
 * 
 * @note Útil para desenvolvimento e testes
 * @note Não afeta o funcionamento do sistema
 */
void debugPrintData();

/**
 * @brief Trata erros de comunicação ESP-NOW
 * 
 * @param result Código de erro retornado pela transmissão
 * 
 * @details Interpreta e registra diferentes tipos 
 * de erros de comunicação
 * 
 * @note Tenta reinicializar comunicação em caso de erro crítico
 * @see setupEspNow()
 */
void handleCommunicationErrors(esp_err_t result);

 /**
  * @brief Callback para status de envio de dados via ESP-NOW
  * 
  * @param mac_addr Endereço MAC do dispositivo de destino
  * @param status Status do envio (sucesso ou falha)
  * 
  * @note Função chamada após cada tentativa de transmissão
  */
 void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
     char macStr[18];
     snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
              mac_addr[0], mac_addr[1], mac_addr[2], 
              mac_addr[3], mac_addr[4], mac_addr[5]);
     
     Serial.printf("Transmissao para %s: %s\n", 
                   macStr, 
                   status == ESP_NOW_SEND_SUCCESS ? "SUCESSO" : "FALHA");
 }
 /**
 * @brief Gera nome de arquivo de log único
 * @return String com nome do arquivo de log
 */
String generateLogFilename() {
  unsigned long timestamp = millis() / 1000;
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "/flight_log_%06lu.csv", timestamp);
  return String(buffer);
}

/**
 * @brief Inicia um novo log de voo
 * @details Cria arquivo CSV com cabeçalho e configura estado de voo
 */
void startNewFlightLog() {
  if (!sdReady) {
      Serial.println("Cartão SD não está pronto!");
      return;
  }

  // Gera nome único para arquivo de log
  currentLogFilename = generateLogFilename();

  // Abre arquivo para escrita
  File file = SD.open(currentLogFilename.c_str(), FILE_WRITE);
  if (file) {
      // Cabeçalho detalhado do CSV
      file.println("Timestamp,AccX,AccY,AccZ,GyroX,GyroY,GyroZ,Temp,Pitch,Roll,Pressao(hPa),Altitude(m)");
      file.close();
      
      // Configura estado de voo
      currentFlightState = FLIGHT_ACTIVE;
      isFlightLogging = true;
      flightStartTime = millis();

      Serial.println("Novo log de voo iniciado: " + currentLogFilename);
  } else {
      Serial.println("Erro ao criar arquivo de log de voo.");
  }
}

/**
 * @brief Finaliza o log de voo atual
 * @details Fecha o arquivo de log e reseta estado de voo
 */
void endFlightLog() {
  if (!isFlightLogging) return;

  // Fecha estado de voo
  currentFlightState = POST_FLIGHT;
  isFlightLogging = false;

  unsigned long flightDuration = (millis() - flightStartTime) / 1000; // em segundos
  
  // Adiciona marcador de fim de voo
  File file = SD.open(currentLogFilename.c_str(), FILE_WRITE);
  if (file) {
      file.println("# FIM DO VOO");
      file.printf("# Duração do Voo: %lu segundos\n", flightDuration);
      file.close();
  }

  Serial.printf("Voo finalizado. Duração: %lu segundos\n", flightDuration);
  currentLogFilename = ""; // Reseta nome do arquivo
}

/**
 * @brief Salva dados no log de voo
 * @details Grava dados atuais dos sensores no arquivo CSV
 */
void logToSD() {
  if (!isFlightLogging || currentLogFilename == "") return;

  File file = SD.open(currentLogFilename.c_str(), FILE_WRITE);
  if (!file) {
      Serial.println("Erro ao abrir arquivo de log.");
      return;
  }

  file.printf("%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
      sensorData.timestamp,
      sensorData.acelerometro.accX,
      sensorData.acelerometro.accY,
      sensorData.acelerometro.accZ,
      sensorData.acelerometro.gyroX,
      sensorData.acelerometro.gyroY,
      sensorData.acelerometro.gyroZ,
      sensorData.acelerometro.temp,
      sensorData.acelerometro.pitch,
      sensorData.acelerometro.roll,
      sensorData.altimetro.pressure,
      sensorData.altimetro.altitude
  );

  file.close();
}

 /**
  * @brief Configura a comunicação ESP-NOW
  * 
  * @details Inicializa o protocolo ESP-NOW, configura o modo de estação 
  * e adiciona o peer de transmissão
  * 
  * @throws Reinicia o sistema em caso de falha crítica
  */
 void setupEspNow() {
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("Falha na inicializacao do ESP-NOW");
        ESP.restart();
        return;
    }

    esp_now_register_send_cb(onDataSent);

    // Configuração de peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, Config::EspNow::broadcastAddress, 6);
    peerInfo.channel = Config::EspNow::CHANNEL;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Falha ao adicionar peer");
    }
}
 /**
  * @brief Tenta inicializar o sensor BMP280 nos dois endereços possíveis
  * 
  * @details Configura BMP280 para os endereços 0x76 e 0x77
  * 
  * @retval true Se o sensor for inicializado com sucesso
  * @retval false Se o sensor falhar na inicialização
  * @note Utiliza o endereço 0x76 como padrão,
  * mas tenta o 0x77 como alternativa
  */
bool initBMP280() {
    // Tenta endereço padrão 0x76
    if (bmp.begin(0x76)) return true;
    
    // Tenta endereço alternativo 0x77
    if (bmp.begin(0x77)) return true;
    
    return false;
}
 /**
  * @brief Inicializa todos os sensores do sistema
  * 
  * @details Configura MPU6050 e BMP280 com parâmetros otimizados
  * 
  * @retval true Se todos os sensores foram inicializados com sucesso
  * @retval false Se algum sensor falhar na inicialização
  * 
  * @throws Loop infinito em caso de falha de sensor
  */
 void setupSensors() {
    // Inicialização do MPU6050
    if (!mpu.begin()) {
        Serial.println("Falha na conexao com MPU6050");
        while(1) delay(10);
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    analogReadResolution(12); // Resolução de 12 bits para ADC
    // Inicialização do BMP280
    if (!initBMP280()) {
        Serial.println("Falha na conexao com BMP280");
        while(1) delay(10);
    }

    // Configurações padrão do BMP280
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Modo de operação
                    Adafruit_BMP280::SAMPLING_X2,     // Oversampling de temperatura 
                    Adafruit_BMP280::SAMPLING_X16,    // Oversampling de pressão
                    Adafruit_BMP280::FILTER_X16,      // Filtro
                    Adafruit_BMP280::STANDBY_MS_500); // Tempo de espera
}
 
 /**
  * @brief Atualiza os dados dos sensores
  * 
  * @details Realiza leitura dos sensores, aplica filtro complementar 
  * e preenche a estrutura de dados de telemetria
  * 
  * @note Limita a taxa de amostragem para evitar sobrecarga
  */
 void updateSensorData() {
    unsigned long currentTime = millis();

    int leituraADC = analogRead(Config::Hardware::ADC_PIN);
    float tensaoPino = (leituraADC / 4095.0) * VREF;
    float tensaoReal = tensaoPino * Config::Hardware::ADC_MULTIPLIER;
    sensorData.tensao.voltage_rocket = tensaoReal;

    // Limita a taxa de leitura
    if (currentTime - lastSensorReadTime < SENSOR_READ_INTERVAL) return;
    
    lastSensorReadTime = currentTime;

    // Leitura do MPU6050
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Cálculo de ângulos com filtro complementar
    float accPitch = atan2(a.acceleration.y, sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.z, 2))) * 180.0 / PI;
    float accRoll = atan2(-a.acceleration.x, a.acceleration.z) * 180.0 / PI;

    static unsigned long lastUpdateTime = 0;
    if (lastUpdateTime == 0) {
        lastUpdateTime = currentTime;
        return; // Ignora primeira leitura
    }
    float dt = (currentTime - lastUpdateTime) / 1000.0;
    lastUpdateTime = currentTime;

    pitch = COMPLEMENTARY_FILTER_ALPHA * (pitch + g.gyro.x * dt * RAD_TO_DEG) + 
            (1 - COMPLEMENTARY_FILTER_ALPHA) * accPitch;
    roll = COMPLEMENTARY_FILTER_ALPHA * (roll + g.gyro.y * dt * RAD_TO_DEG) + 
           (1 - COMPLEMENTARY_FILTER_ALPHA) * accRoll;

    // Preenchimento da estrutura de dados
    sensorData.acelerometro = {
        a.acceleration.x, a.acceleration.y, a.acceleration.z,
        g.gyro.x, g.gyro.y, g.gyro.z,
        temp.temperature,
        pitch, roll
    };

    // Leitura do BMP280
    sensorData.altimetro = {
        bmp.readPressure() / 100.0f,  // Pressão em hPa
        bmp.readAltitude(1013.25)     // Altitude baseada na pressão ao nível do mar
    };

    sensorData.timestamp = currentTime;

    // sensorData.gps = {
    //     gps.location.lat(),
    //     gps.location.lng(),
    //     gps.altitude.meters(),
    //     gps.date.year(),
    //     gps.date.month(),
    //     gps.date.day(),
    //     gps.time.hour(),
    //     gps.time.minute(),
    //     gps.time.second()
    // };

    sensorData.gps = {
        000100.0, // Substitua por gps.location.lat() se GPS estiver implementado
        001100.0, // Substitua por gps.location.lng() se GPS estiver implementado
        0.11,      // Substitua por gps.altitude.meters() se GPS estiver implementado
        2025,        // Substitua por gps.date.year() se GPS estiver implementado
        6,        // Substitua por gps.date.month() se GPS estiver implementado
        2,        // Substitua por gps.date.day() se GPS estiver implementado
        20,        // Substitua por gps.time.hour() se GPS estiver implementado
        30,        // Substitua por gps.time.minute() se GPS estiver implementado
        49         // Substitua por gps.time.second() se GPS estiver implementado
    };

}

 /**
  * @brief Transmite dados de telemetria via ESP-NOW
  * 
  * @details Envia pacote de dados para o endereço de broadcast
  * 
  * @note Limita a taxa de transmissão
  */
void transmitData() {
    unsigned long currentTime = millis();
    
    // Limita a taxa de transmissão
    if (currentTime - lastTransmissionTime < TRANSMISSION_INTERVAL) return;
    
    lastTransmissionTime = currentTime;

    // Verifica se o peer existe antes de enviar
    if (!esp_now_is_peer_exist(Config::EspNow::broadcastAddress)) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, Config::EspNow::broadcastAddress, 6);
        peerInfo.channel = Config::EspNow::CHANNEL;
        peerInfo.encrypt = false;

        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Falha ao adicionar peer para transmissão");
            return;
        }
    }

    esp_err_t result = esp_now_send(
        Config::EspNow::broadcastAddress, 
        reinterpret_cast<uint8_t*>(&sensorData), 
        sizeof(SensorData)
    );

    handleCommunicationErrors(result);
}
 
 /**
  * @brief Trata erros de comunicação ESP-NOW
  * 
  * @param result Código de erro retornado pela transmissão
  * 
  * @details Interpreta e registra diferentes tipos de erros de comunicação
  */
 void handleCommunicationErrors(esp_err_t result) {
    switch(result) {
        case ESP_OK:
            Serial.println("ENVIADO com sucesso");
            break;
        case ESP_ERR_ESPNOW_NOT_INIT:
            Serial.println("ESP-NOW nao inicializado");
            setupEspNow();
            break;
        case ESP_ERR_ESPNOW_ARG:
            Serial.println("Argumento invalido");
            break;
        case ESP_ERR_ESPNOW_NO_MEM:
            Serial.println("Sem memoria");
            break;
        default:
            Serial.printf("Erro desconhecido: %d\n", result);
    }
}
 
 /**
  * @brief Imprime dados de telemetria para depuração
  * 
  * @details Exibe informações detalhadas dos sensores no console serial
  * 
  * @note Útil para desenvolvimento e testes
  */
 void debugPrintData() {
    Serial.println("===== TELEMETRIA =====");
    Serial.printf("Aceleracao: X=%.2f, Y=%.2f, Z=%.2f m/s²\n", 
        sensorData.acelerometro.accX, 
        sensorData.acelerometro.accY, 
        sensorData.acelerometro.accZ);
    
    Serial.printf("Giroscopio: X=%.2f, Y=%.2f, Z=%.2f rad/s\n", 
        sensorData.acelerometro.gyroX, 
        sensorData.acelerometro.gyroY, 
        sensorData.acelerometro.gyroZ);

    Serial.printf("Orientacao: Pitch=%.2f°, Roll=%.2f°\n", 
        sensorData.acelerometro.pitch, 
        sensorData.acelerometro.roll);

    Serial.printf("Temperatura: %.2f °C\n", sensorData.acelerometro.temp);
    Serial.printf("Pressao: %.2f hPa\n", sensorData.altimetro.pressure);
    Serial.printf("Altitude: %.2f m\n", sensorData.altimetro.altitude);
    Serial.printf("Timestamp: %lu ms\n", sensorData.timestamp);
    Serial.println("====================\n");
}
 

/**
 * @brief Callback para recepção de comandos de controle
 * @param mac Endereço MAC do remetente
 * @param incomingData Dados recebidos
 * @param len Tamanho dos dados
 */
void onEspNowReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(ControlCommand)) {
      ControlCommand command;
      memcpy(&command, incomingData, sizeof(ControlCommand));

      switch(command.type) {
          case START_FLIGHT:
              if (currentFlightState == GROUND_STATE) {
                  startNewFlightLog();
              }
              break;
          
          case END_FLIGHT:
              if (currentFlightState == FLIGHT_ACTIVE) {
                  endFlightLog();
              }
              break;
          
          default:
              Serial.println("Comando não reconhecido");
              break;
      }
  }
}


/**
 * @brief Configuração do sistema de telemetria
 */
void setup() {
  Serial.begin(Config::Hardware::BAUD_RATE);
  while (!Serial) delay(10);


//   // Inicialização do SD Card
//   if (!SD.begin(Config::Hardware::SD_CS_PIN)) {
//       Serial.println("Falha na inicialização do SD Card");
//       sdReady = false;
//   } else {
//       sdReady = true;
//       Serial.println("Cartão SD inicializado com sucesso");
//   }

  // Inicialização do barramento I2C
  Wire.begin();
  
  // Configurações de rede e comunicação
  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.print("MAC da ESP32: ");
  Serial.println(WiFi.macAddress());
  
  // Inicialização do ESP-NOW
  if (esp_now_init() != ESP_OK) {
      Serial.println("Erro ao inicializar ESP-NOW");
      ESP.restart();
  }

  // Registra callback de recepção de dados
  esp_now_register_recv_cb(onEspNowReceive);

  setupEspNow();
  // Inicialização dos sensores
  setupSensors();

  Serial.println("Sistema de Telemetria Inicializado");
}

/**
 * @brief Laço principal de execução
 */
void loop() {
  updateSensorData();
  transmitData();
  
  if (isFlightLogging) {
      logToSD();
      debugPrintData();
  }

  delay(100);  // Pequeno atraso para estabilidade
}
 