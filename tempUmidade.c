// --- Configuração da Porta I2C 1 da BitDogLab---
//#define I2C_PORT i2c1
//const uint I2C_SDA_PIN = 2;
//const uint I2C_SCL_PIN = 3;

// Inclusão de bibliotecas padrão e específicas da Raspberry Pi Pico
#include <stdio.h>
#include "pico/stdlib.h"         // Funções básicas do Pico (GPIO, delay, etc)
#include "hardware/i2c.h"        // Biblioteca para comunicação I2C
#include "aht10.h"               // Biblioteca personalizada para o sensor AHT10
#include "inc/ssd1306.h"         // Biblioteca OLED
#include "inc/ssd1306_fonts.h"   // Fontes para o display OLED

// Definição da porta I2C e pinos utilizados
#define I2C_PORT i2c0
const uint I2C_SDA_PIN = 0;
const uint I2C_SCL_PIN = 1;

// Definição dos pinos dos LEDs e buzzer
#define LED_AZUL      12
#define LED_VERMELHO  13
#define LED_VERDE     11
#define BUZZER        10

// Variável que controla a próxima ativação do buzzer
absolute_time_t proxima_ativacao_buzzer;

int main() {
    stdio_init_all(); // Inicializa a comunicação serial para debug via USB

    // Inicializa a porta I2C com velocidade de 100 kHz
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // Configura SDA como função I2C
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C); // Configura SCL como função I2C
    gpio_pull_up(I2C_SDA_PIN);  // Ativa pull-up necessário na linha SDA
    gpio_pull_up(I2C_SCL_PIN);  // Ativa pull-up necessário na linha SCL

    // Inicializa os pinos GPIO para LEDs e buzzer como saída
    gpio_init(LED_AZUL);     gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_init(LED_VERMELHO); gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(LED_VERDE);    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(BUZZER);       gpio_set_dir(BUZZER, GPIO_OUT);

    // Inicializa o sensor AHT10 e verifica se foi bem-sucedido
    if (!aht10_init(I2C_PORT)) {
        printf("Erro ao inicializar AHT10\n");
        while (1); // Loop infinito se houver erro
    }

    // Inicializa o display OLED
    ssd1306_Init();           // Inicializa o driver
    ssd1306_Fill(Black);      // Limpa a tela
    ssd1306_UpdateScreen();   // Atualiza a tela

    // Define tempo inicial para controle do buzzer
    proxima_ativacao_buzzer = get_absolute_time();

    // Loop principal do programa
    while (1) {
        aht10_dado_t dado;  // Estrutura que armazena temperatura e umidade

        // Se leitura do sensor for bem-sucedida
        if (aht10_ler_dado(I2C_PORT, &dado)) {
            char buffer[32]; // Buffer para formatar strings

            // === Atualiza informações no display OLED ===
            ssd1306_Fill(Black); // Limpa tela
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString("MONITOR AHT10", Font_7x10, White);

            // Exibe temperatura
            snprintf(buffer, sizeof(buffer), "TEMP: %.1f C", dado.temperatura);
            ssd1306_SetCursor(0, 12);
            ssd1306_WriteString(buffer, Font_7x10, White);

            // Exibe umidade
            snprintf(buffer, sizeof(buffer), "UMID: %.1f %%", dado.umidade);
            ssd1306_SetCursor(0, 24);
            ssd1306_WriteString(buffer, Font_7x10, White);

            // Alerta na tela dependendo das condições ambientais
            if (dado.umidade > 70.0f || dado.temperatura < 20.0f) {
                ssd1306_SetCursor(0, 36);
                ssd1306_WriteString("ALERTA: Condicao critica!", Font_6x8, White);
            } else {
                ssd1306_SetCursor(0, 36);
                ssd1306_WriteString("Ambiente OK", Font_6x8, White);
            }

            ssd1306_UpdateScreen(); // Atualiza display com os dados

            // Exibe informações via serial (debug)
            printf("Temperatura: %.2f C | Umidade: %.2f %%\n", dado.temperatura, dado.umidade);

            // === Lógica para acionar LEDs ===
            bool umidade_alta = dado.umidade > 70.0f;

            // LED azul: temperatura abaixo de zero
            gpio_put(LED_AZUL, dado.temperatura < 0.0f);

            // LED vermelho: temperatura ≥ 35°C se umidade não estiver alta
            if (!umidade_alta && dado.temperatura >= 35.0f)
                gpio_put(LED_VERMELHO, 1);
            else if (!umidade_alta)
                gpio_put(LED_VERMELHO, 0);

            // LED verde: ambiente ideal entre 20°C e 30°C
            gpio_put(LED_VERDE, dado.temperatura >= 20.0f && dado.temperatura <= 30.0f);

            // === Controle do buzzer ===
            if (dado.temperatura >= 40.0f) {
                absolute_time_t agora = get_absolute_time();
                // Só ativa se já passou o tempo mínimo desde a última ativação
                if (absolute_time_diff_us(proxima_ativacao_buzzer, agora) <= 0) {
                    gpio_put(BUZZER, 1);
                    sleep_ms(200);       // Toca por 200ms
                    gpio_put(BUZZER, 0);
                    proxima_ativacao_buzzer = make_timeout_time_ms(10000); // Aguarda 10s para próxima ativação
                }
            }

            // === Alerta para umidade alta: piscar LED vermelho ===
            if (umidade_alta) {
                for (int i = 0; i < 2; i++) {
                    gpio_put(LED_VERMELHO, 1);
                    sleep_ms(500);
                    gpio_put(LED_VERMELHO, 0);
                    sleep_ms(500);
                }
                continue; // Pula o restante do loop e volta a ler sensor
            }

        } else {
            // Se leitura falhar, exibe mensagem de erro
            printf("Erro ao ler sensor!\n");
        }

        sleep_ms(2000); // Aguarda 2 segundos antes de nova leitura
    }

    return 0; // Nunca alcançado, pois o loop é infinito
}
