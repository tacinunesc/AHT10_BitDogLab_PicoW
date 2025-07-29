// --- Configuração da Porta I2C 1 da BitDogLab---
//#define I2C_PORT i2c1
//const uint I2C_SDA_PIN = 2;
//const uint I2C_SCL_PIN = 3;

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "aht10.h"
#include "inc/ssd1306.h"
#include "inc/ssd1306_fonts.h"

// Configuração da Porta I2C
#define I2C_PORT i2c0
const uint I2C_SDA_PIN = 0;
const uint I2C_SCL_PIN = 1;

#define LED_AZUL      12
#define LED_VERMELHO  13
#define LED_VERDE     11
#define BUZZER        10

absolute_time_t proxima_ativacao_buzzer;

int main() {
    stdio_init_all();

    // Inicializa I2C
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Inicializa GPIOs
    gpio_init(LED_AZUL);     gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_init(LED_VERMELHO); gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(LED_VERDE);    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(BUZZER);       gpio_set_dir(BUZZER, GPIO_OUT);

    // Inicializa sensor AHT10
    if (!aht10_init(I2C_PORT)) {
        printf("Erro ao inicializar AHT10\n");
        while (1);
    }

    // Inicializa Display OLED
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    proxima_ativacao_buzzer = get_absolute_time();

    while (1) {
        aht10_dado_t dado;

        if (aht10_ler_dado(I2C_PORT, &dado)) {
            char buffer[32];

            // === DISPLAY OLED ===
            ssd1306_Fill(Black);
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString("MONITOR AHT10", Font_7x10, White);

            snprintf(buffer, sizeof(buffer), "TEMP: %.1f C", dado.temperatura);
            ssd1306_SetCursor(0, 12);
            ssd1306_WriteString(buffer, Font_7x10, White);

            snprintf(buffer, sizeof(buffer), "UMID: %.1f %%", dado.umidade);
            ssd1306_SetCursor(0, 24);
            ssd1306_WriteString(buffer, Font_7x10, White);

            // === ALERTA NA TELA ===
            if (dado.umidade > 70.0f || dado.temperatura < 20.0f) {
                ssd1306_SetCursor(0, 36);
                ssd1306_WriteString("ALERTA: Condicao critica!", Font_6x8, White);
            } else {
                ssd1306_SetCursor(0, 36);
                ssd1306_WriteString("Ambiente OK", Font_6x8, White);
            }

            ssd1306_UpdateScreen();

            // === PRINT SERIAL ===
            printf("Temperatura: %.2f C | Umidade: %.2f %%\n", dado.temperatura, dado.umidade);

            // === AÇÕES ===
            bool umidade_alta = dado.umidade > 70.0f;

            gpio_put(LED_AZUL, dado.temperatura < 0.0f);

            if (!umidade_alta && dado.temperatura >= 35.0f)
                gpio_put(LED_VERMELHO, 1);
            else if (!umidade_alta)
                gpio_put(LED_VERMELHO, 0);

            gpio_put(LED_VERDE, dado.temperatura >= 20.0f && dado.temperatura <= 30.0f);

            // Buzzer: temperatura ≥ 40 °C
            if (dado.temperatura >= 40.0f) {
                absolute_time_t agora = get_absolute_time();
                if (absolute_time_diff_us(proxima_ativacao_buzzer, agora) <= 0) {
                    gpio_put(BUZZER, 1);
                    sleep_ms(200);
                    gpio_put(BUZZER, 0);
                    proxima_ativacao_buzzer = make_timeout_time_ms(10000);
                }
            }

            // Piscar LED vermelho se umidade > 70%
            if (umidade_alta) {
                for (int i = 0; i < 2; i++) {
                    gpio_put(LED_VERMELHO, 1);
                    sleep_ms(500);
                    gpio_put(LED_VERMELHO, 0);
                    sleep_ms(500);
                }
                continue;
            }

        } else {
            printf("Erro ao ler sensor!\n");
        }

        sleep_ms(2000);
    }

    return 0;
}
