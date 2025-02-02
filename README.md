# Tarefa I - Unidade IV
Repositório criado a fim de armazenar a tarefa realizada para consolidar a compreensão dos conceitos relacionados ao uso de interrupções no microcontrolador RP2040 e explorar as funcionalidades da placa de desenvolvimento BitDogLab.

__Autora:__ Ana Karolina Disigant Reis <br>
__Vídeo da aplicação:__

## Objetivos
• Compreender o funcionamento e a aplicação de interrupções em microcontroladores;<br>
• Identificar e corrigir o fenômeno do bouncing em botões por meio de debouncing via software; <br>
• Manipular e controlar LEDs comuns e LEDs endereçáveis WS2812; <br>
• Fixar o estudo do uso de resistores de pull-up internos em botões de acionamento. <br>
• Desenvolver um projeto funcional que combine hardware e software.<br>

## Descrição do Projeto
__Utilização obrigatória__ <br>
Matriz 5x5 de LEDs (endereçáveis) WS2812, conectada à GPIO 7; <br>
LED RGB, com os pinos conectados às GPIOs (11, 12 e 13); <br>
Botão A conectado à GPIO 5; <br>
Botão B conectado à GPIO 6.

__Funcionalidades do Projeto__
1. O LED vermelho do LED RGB deve piscar continuamente 5 vezes por segundo;
```c
void flash_red_led() {
    const uint delay_ms = 100;  // Tempo de 100 ms para garantir 5 piscadas por segundo

    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN_RED, 1);  // Liga o LED vermelho
        sleep_ms(delay_ms);        // Espera pelo tempo configurado
        gpio_put(LED_PIN_RED, 0);  // Desliga o LED vermelho
        sleep_ms(delay_ms);        // Espera novamente
    }
}
```
2. O botão A deve incrementar o número exibido na matriz de LEDs cada vez que for pressionado;
3. O botão B deve decrementar o número exibido na matriz de LEDs cada vez que for pressionado;
4. Os LEDs WS2812 devem ser usados para criar efeitos visuais representando números de 0 a 9.<br>

__Requisitos do Projeto__

__1. Uso de interrupções:__ Todas as funcionalidades relacionadas aos botões devem ser implementadas
utilizando rotinas de interrupção (IRQ).<br>
__1.1. Registro da interrupção dos botões (no `main()`):__
```c
gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, handle_button_irq);
gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, handle_button_irq);
```
- Configuração dos botões `BUTTON_A_PIN` e `BUTTON_B_PIN` para chamar `handle_button_irq()` quando um evento de borda de descida (`GPIO_IRQ_EDGE_FALL`, ou seja, quando o botão é pressionado) ocorre.<br>

__1.2. A função de interrupção (`handle_button_irq()`):__
```c
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
```
- Essa função é chamada automaticamente sempre que um dos botões gera uma interrupção;  
- O código verifica se o botão foi pressionado e aplica um __debounce__ para evitar múltiplas detecções erradas.  
- Define as flags `is_button_a_pressed` e `is_button_b_pressed` para serem processadas no loop principal.<br>

__2. Debouncing:__ É obrigatório implementar o tratamento do bouncing dos botões via software.<br>

__2.1. Uso de um timestamp para debounce:__
```c
uint64_t last_button_a_timestamp = 0;
uint64_t last_button_b_timestamp = 0;
#define DEBOUNCE_DELAY_MS 200
```
- As variáveis `last_button_a_timestamp` e `last_button_b_timestamp` armazenam o **último tempo** em que cada botão foi pressionado.  
- O valor `DEBOUNCE_DELAY_MS` define o **tempo mínimo entre duas detecções** para evitar múltiplos acionamentos causados pelo bouncing.

__2.2. Verificação de debounce dentro da interrupção:__
```c
void handle_button_irq(uint gpio, uint32_t events) {
    uint64_t current_time = time_us_64();

    // Verifica se o botão A foi pressionado
    if (gpio == BUTTON_A_PIN && current_time - last_button_a_timestamp > DEBOUNCE_DELAY_MS * 1000) {
        if (!gpio_get(BUTTON_A_PIN)) { // Confirma que o botão realmente foi pressionado
            is_button_a_pressed = true;
            last_button_a_timestamp = current_time; // Atualiza o timestamp
        }
    } 
    // Verifica se o botão B foi pressionado
    else if (gpio == BUTTON_B_PIN && current_time - last_button_b_timestamp > DEBOUNCE_DELAY_MS * 1000) {
        if (!gpio_get(BUTTON_B_PIN)) { // Confirma que o botão realmente foi pressionado
            is_button_b_pressed = true;
            last_button_b_timestamp = current_time; // Atualiza o timestamp
        }
    }
}
```
__Como esse código trata o bouncing?__<br>
✅ __Filtro de tempo:__ O código verifica se já passaram pelo menos __200 ms__ desde a última ativação do botão antes de processar uma nova pressão.  
✅ __Confirmação da pressão:__ Antes de marcar o botão como pressionado (`is_button_a_pressed = true;`), o código verifica se o botão ainda está realmente pressionado (`!gpio_get(BUTTON_A_PIN)`). Isso reduz falsos acionamentos.  
✅ __Implementado dentro da interrupção:__ O debounce é tratado __diretamente na interrupção__, evitando a necessidade de loops adicionais no `main()`.

Com esse mecanismo, o código garante que um __único pressionamento seja registrado corretamente__, mesmo que o botão gere sinais oscilantes devido ao bouncing físico.<br>

__3. Controle de LEDs:__ O projeto deve incluir o uso de LEDs comuns e LEDs WS2812, demonstrando o
domínio de diferentes tipos de controle.<br>

__3.1 Controle de LEDs comuns (LEDs individuais)__
Os LEDs comuns estão conectados a pinos específicos do Raspberry Pi Pico e são controlados via GPIO:

__Definição dos pinos dos LEDs comuns__ 
```c
#define LED_PIN_RED    13
#define LED_PIN_GREEN  11
#define LED_PIN_BLUE   12
```

__Inicialização dos LEDs comuns__
```c
gpio_init(LED_PIN_RED);
gpio_set_dir(LED_PIN_RED, GPIO_OUT);
gpio_init(LED_PIN_GREEN);
gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
gpio_init(LED_PIN_BLUE);
gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
```
- Cada LED é inicializado e configurado como saída (`GPIO_OUT`).
- Isso permite controlar cada LED individualmente com `gpio_put()`.

__Função para piscar o LED vermelho__
```c
void flash_red_led() {
    const uint delay_ms = 100;  // Tempo de 100 ms para garantir 5 piscadas por segundo

    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN_RED, 1);  // Liga o LED vermelho
        sleep_ms(delay_ms);        // Espera pelo tempo configurado
        gpio_put(LED_PIN_RED, 0);  // Desliga o LED vermelho
        sleep_ms(delay_ms);        // Espera novamente
    }
}
```

__3.2. Controle dos LEDs WS2812 (Matriz de LEDs)__
Os LEDs WS2812 são controlados via **PIO (Programmable I/O),** permitindo manipulação precisa de cores e padrões.

__3.3. Definição do pino dos LEDs WS2812__
```c
#define WS2812_PIN 7
```

__3.4. Inicialização do PIO para os LEDs WS2812__
```c
PIO pio = pio0;
uint sm = 0;
uint offset = pio_add_program(pio, &ws2812_program);
ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
```
- O __PIO__ é configurado para controlar os LEDs WS2812.
- `ws2812_program_init()` configura o PIO para enviar sinais compatíveis com os LEDs WS2812.

__3. 5. Exibição de números na matriz de LEDs__
```c
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
```
- A função recebe um número e exibe o __padrão correspondente na matriz__.
- Os LEDs WS2812 são controlados enviando __códigos de cor via PIO__.

__4. Organização do código:__ O código deve estar bem estruturado e comentado para facilitar o
entendimento. ✅
