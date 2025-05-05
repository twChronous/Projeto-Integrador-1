# 🚀 Telemetria para Foguete d'Água - Projeto Integrador 1

## 📌 Visão Geral do Projeto

Este repositório contém o sistema de telemetria desenvolvido para um foguete d'água como parte do Projeto Integrador 1 da faculdade. O objetivo principal é criar um sistema de monitoramento em tempo real para coletar e transmitir dados durante o voo do foguete.

## 🎯 Objetivos do Projeto

- Desenvolver um sistema de telemetria robusto e preciso
- Coletar dados de:
  - Altitude
  - Aceleração
  - Orientação
  - Temperatura
- Transmitir dados em tempo real
- Registrar informações para análise pós-voo

## 🛰️ Componentes do Sistema de Telemetria

### Hardware
- Microcontrolador: ESP32
- Sensores:
  - Acelerômetro/Giroscópio
  - Altímetro
  - Sensor de Temperatura
- Comunicação: ESP-NOW / WiFi

### Software
- Firmware em C++
- Protocolo de comunicação sem fio
- Sistema de registro de dados
- Interface de visualização de dados

## 📂 Estrutura do Repositório

```
projeto-foguete-telemetria/
│
├── include/               # Arquivos de cabeçalho
│   ├── SensorStructs.h    # Estruturas de dados dos sensores
│   └── Config.h           # Configurações do sistema
│
├── src/                   # Código-fonte
│   ├── main.cpp           # Lógica principal
│   ├── sensors.cpp        # Implementação dos sensores
│   └── communication.cpp  # Módulo de comunicação
│
├── docs/                  # Documentação do projeto
│
├── resources/             # Arquivos de imagem
│
├── test/                  # Testes unitários e de integração
│
└── README.md              # Este arquivo
```

## 🔧 Tecnologias Utilizadas

- Linguagem: C++
- Plataforma: PlatformIO / Arduino
- Microcontrolador: ESP32
- Comunicação: ESP-NOW, WiFi
- Sensores: MPU6050, BMP280

<div style="text-align: center;">
    <img src="./resources/base.schematic.png" alt="Esquematico do projeto" width="600">
</div>

## 🚀 Funcionalidades Principais

1. **Coleta de Dados**
   - Altitude em tempo real
   - Aceleração nos três eixos
   - Orientação (pitch, roll)
   - Temperatura

2. **Comunicação**
   - Transmissão sem fio de telemetria
   - Alcance de até 1 km
   - Baixa latência

3. **Registro de Dados**
   - Armazenamento em cartão SD
   - Formato CSV para análise
   - Carimbo de tempo preciso

## 📊 Métricas e Precisão

- Precisão de altitude: ±0.5 metros
- Taxa de amostragem: 10 Hz
- Precisão de aceleração: ±0.1 m/s²
- Intervalo de temperatura: -10°C a 85°C

## 🔬 Desafios do Projeto

- Resistência a altas vibrações
- Comunicação em ambientes externos
- Garantir precisão dos sensores

## 🛠️ Configuração e Instalação

### Pré-requisitos
- PlatformIO
- Biblioteca ESP32
- Bibliotecas de sensores

### Passos de Instalação
1. Clone o repositório
2. Abra no PlatformIO
3. Instale dependências
4. Compile e faça upload


## 🤝 Contribuição

1. Faça um fork do projeto
2. Crie sua branch de feature
3. Commit suas alterações
4. Push para a branch
5. Abra um Pull Request

## 📋 Documentação Adicional

- [Documentação do projeto](https://twchronous.github.io/Projeto-Integrador-1/md_readme.html)

## 📄 Licença

[GPL](./LICENSE)