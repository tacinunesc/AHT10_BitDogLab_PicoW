# 🌡 Sensor de Temperatura e Umidade AHT10 atuando com Pico W (BitDogLab)
![Linguagem](https://img.shields.io/badge/Linguagem-C-blue.svg)
![Plataforma](https://img.shields.io/badge/Plataforma-Raspberry%20Pi%20Pico-purple.svg)
![Sensor](https://img.shields.io/badge/Sensor-AHT10-yellow.svg)


## Sobre o Projeto
Este código foi desenvolvido para um sistema de monitoramento de Temperatura e Umidade com o Raspberry Pi Pico, utilizando os seguintes componentes:

<div align="center">
  <img src="img/led verde.jpg "  alt="Controle" width="30%">
  <img src="img/led vermelho.jpg "  alt="Controle" width="30%">
</div>

Sensor AHT10 para medir luminosidade via I2C

Display OLED SSD1306 para exibir informações

LEDs RGB para indicar níveis temperatura e umidade elevadas ou não


## 🛠️ Estrutura do projeto
- tempUmidade.c – Programa principal em C que faz leitura da temperatura e umidade do ambiente, com isso esta informação é exibida no porta serial e no visor oled da BitDogLab
- aht10.c - Onde fica as definições do sensor de temperatura e umidade
- Pasta inc - Onde esta localizada as informações da oled
- CMakeLists.txt – Configuração do build usando o Pico SDK

- ## 🔌 Requisitos
Hardware:

- Raspberry Pi Pico W
- Sensor de temperatura e umidade (AHT10)

## ⚙️ Como usar
1- Clone o repositorio

2- Deploy no Pico W
 - Segure o botão BOOTSEL do Pico W e conecte-o ao PC
 - Clique no botão run no Vscode ou arraste o arquivo .u2 para dentro do disco removível que aparecer
 - O Pico irá reiniciar executando o firmware

## 🔧 Funcionamento do Código
O programa realiza as seguintes ações:

1. Inicialização dos periféricos
- Configura o barramento I2C0 (pinos GPIO 0 e 1)
- Inicializa o sensor AHT10 e o display OLED SSD1306
- Configura os pinos dos LEDs RGB
  
2. Leitura de Temperatura e Umidade

- Lê a temperatura e umidade com o sensor AHT10
- Exibe os valores no display OLED
- Acende um LED correspondente ao nível de temperatura e umidade:

   * 🔵 Azul: temperatura entre 0° a 19°

   * 🟢 Verde: temperatura entre 20° a 30°

   * 🔴 Vermelho: umidade > 70% pisca o led vermelho
   * Temperatura acima de 40° ativa o buzzer

## 📦 Dependências

aht10.h para o sensor de temperatura e umidade

ssd1306.h e ssd1306_fonts.h para o display OLED
