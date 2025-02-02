#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

// Definições de pinos
#define LED_PIN_RED    13
#define LED_PIN_GREEN  11
#define LED_PIN_BLUE   12
#define BUTTON_A_PIN   5
#define BUTTON_B_PIN   6
#define WS2812_PIN     7

// Constantes
#define DEBOUNCE_DELAY_MS 200  // Tempo de debounce mais longo para melhorar a detecção
#define BLINK_DELAY_MS    100  // Tempo de piscar do LED em milissegundos
#define MATRIX_SIZE      5     // Tamanho da matriz de LEDs (5x5)

// Variáveis globais
int current_number = 0;  // Número exibido na matriz
bool is_button_a_pressed = false;
bool is_button_b_pressed = false;
uint64_t last_button_a_timestamp = 0;
uint64_t last_button_b_timestamp = 0;

// Variáveis de controle de cor do LED
uint8_t red_intensity = 50;   // Intensidade do vermelho (0 a 255)
uint8_t green_intensity = 0;   // Intensidade do verde (0 a 255)
uint8_t blue_intensity = 60;  // Intensidade do azul (0 a 255)

// Padrões de LEDs para números de 0 a 9 (5x5)
const bool number_patterns[10][MATRIX_SIZE * MATRIX_SIZE] = {
    {1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1}, // 0
    {0,1,1,1,0, 0,0,1,0,0, 0,0,1,0,0, 0,1,1,0,0, 0,0,1,0,0}, // 1
    {1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 2
    {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 3
    {1,0,0,0,0, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1}, // 4
    {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1}, // 5
    {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1}, // 6
    {0,0,0,0,1, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,1,0, 1,1,1,1,1}, // 7
    {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}, // 8
    {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}  // 9
};

// Função para exibir o número na matriz
void show_number(int number, PIO pio, uint sm) {
    printf("Exibindo número: %d\n", number);  // Depuração
    const bool *pattern = number_patterns[number];      
    uint32_t led_color = (green_intensity << 24) | (red_intensity << 16) | (blue_intensity << 8); // Cor padrão para os LEDs acesos

    // Percorre todos os LEDs da matriz
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        // Se o LED deve ser aceso, aplica a cor
        if (pattern[i]) {
            pio_sm_put_blocking(pio, sm, led_color);
        } else {
            pio_sm_put_blocking(pio, sm, 0); // LED apagado
        }
    }
}

// Função para piscar o LED vermelho
void flash_red_led() {
    const uint delay_ms = 100;  // Tempo de 100 ms para garantir 5 piscadas por segundo

    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN_RED, 1);  // Liga o LED vermelho
        sleep_ms(delay_ms);        // Espera pelo tempo configurado
        gpio_put(LED_PIN_RED, 0);  // Desliga o LED vermelho
        sleep_ms(delay_ms);        // Espera novamente
    }
}

// Função de interrupção para os botões
void handle_button_irq(uint gpio, uint32_t events) {
    uint64_t current_time = time_us_64();

    // Verifica se o botão A foi pressionado
    if (gpio == BUTTON_A_PIN && current_time - last_button_a_timestamp > DEBOUNCE_DELAY_MS * 1000) {
        if (!gpio_get(BUTTON_A_PIN)) { // Verifica se o botão foi pressionado
            is_button_a_pressed = true;
            last_button_a_timestamp = current_time; // Atualiza o timestamp
        }
    } 
    
    // Verifica se o botão B foi pressionado
    else if (gpio == BUTTON_B_PIN && current_time - last_button_b_timestamp > DEBOUNCE_DELAY_MS * 1000) {
        if (!gpio_get(BUTTON_B_PIN)) { // Verifica se o botão foi pressionado
            is_button_b_pressed = true;
            last_button_b_timestamp = current_time; // Atualiza o timestamp
        }
    }
}

// Função principal
int main() {
    // Configura LEDs comuns
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);

    // Configura botões com interrupções
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, handle_button_irq);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, handle_button_irq);

    // Inicializa o PIO para controlar os LEDs WS2812
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    stdio_init_all();    

    // Exibe o número 0 logo após a inicialização
    show_number(0, pio, sm);

    while (true) {
        flash_red_led(BLINK_DELAY_MS);  // Pisca o LED vermelho

        // Processamento do botão A
        if (is_button_a_pressed) {
            is_button_a_pressed = false;  // Limpa o status do botão
            if (current_number == 9) {
                current_number = 0;
            } else {
                current_number++;
            }
            show_number(current_number, pio, sm);
        }

        // Processamento do botão B
        if (is_button_b_pressed) {
            is_button_b_pressed = false;  // Limpa o status do botão
            if (current_number == 0) {
                current_number = 9;
            } else {
                current_number--;
            }
            show_number(current_number, pio, sm);
        }
    }
    return 0;
}