#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "libs/ssd1306.h"
#include "hardware/adc.h"
#include <string.h>
#include "hardware/pwm.h"

// ========================================================================
// DEFINIÇÕES DE HARDWARE
// ========================================================================
#define I2C_PORT i2c1
#define PINO_SCL 14
#define PINO_SDA 15
#define SW 22          // Botão do joystick
#define VRX 27         // Eixo X do joystick
#define VRY 26         // Eixo Y do joystick

#define BUTTON_A 5     // Botão A
#define BUTTON_B 6     // Botão B
#define LED_RED 13     // LED vermelho
#define LED_GREEN 11   // LED verde

#define ADC_CHANNEL_X 1
#define ADC_CHANNEL_Y 0
#define DEADZONE 300
#define MENU_ITEMS 7  
#define MENU_Y_START 12
#define MENU_Y_STEP 12

typedef struct {
    uint8_t index;
    absolute_time_t last_move;
    bool first_move;
} MenuControl;

ssd1306_t disp;
MenuControl menu = {0, 0, true};
bool in_program = false;

// ========================================================================
// PROTÓTIPOS
// ========================================================================
void inicializa_hardware();
void update_display();
void handle_joystick();
void handle_button();
bool button_pressed();
bool button_a_pressed();
bool button_b_pressed();
void draw_truth_table(bool a, bool b, bool result, const char* gate_name);
void reset_leds();

// ========================================================================
// FUNÇÃO DE ATUALIZAÇÃO DO DISPLAY (CORRIGIDA)
// ========================================================================
void update_display() {
    ssd1306_clear(&disp);
    
    const char* titulo = "PORTAS LOGICAS";
    uint8_t titulo_x = (128 - (strlen(titulo) * 6)) / 2; // Parênteses corrigido
    ssd1306_draw_string(&disp, titulo_x, 2, 1, titulo);

    const char* items[MENU_ITEMS] = {"AND", "OR", "NOT", "NAND", "NOR", "XOR", "XNOR"};
    const uint8_t col1_x = 10;
    const uint8_t col2_x = 74;
    uint8_t y_start = 20;

    for(uint8_t i = 0; i < MENU_ITEMS; i++) {
        uint8_t x = (i < 4) ? col1_x : col2_x;
        uint8_t y = y_start + ((i % 4) * 12);

        ssd1306_draw_string(&disp, x, y, 1, items[i]);
    }

    uint8_t sel_x = (menu.index < 4) ? col1_x - 8 : col2_x - 8;
    uint8_t sel_y = y_start + ((menu.index % 4) * 12) - 4;
    ssd1306_draw_empty_square(&disp, sel_x, sel_y, 56, 16);
    
    ssd1306_show(&disp);
}

// ========================================================================
// INICIALIZAÇÃO DE HARDWARE
// ========================================================================
void inicializa_hardware() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);

    i2c_init(I2C_PORT, 100000);
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PINO_SCL);
    gpio_pull_up(PINO_SDA);

    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);
    ssd1306_clear(&disp);
    ssd1306_show(&disp);

    // Configuração dos botões
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Configuração dos LEDs
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
// Configuração dos LEDs como PWM
gpio_set_function(LED_RED, GPIO_FUNC_PWM);
gpio_set_function(LED_GREEN, GPIO_FUNC_PWM);

// Configuração dos LEDs como PWM
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    gpio_set_function(LED_GREEN, GPIO_FUNC_PWM);

    // Configurar slices PWM
    uint slice_red = pwm_gpio_to_slice_num(LED_RED);
    uint slice_green = pwm_gpio_to_slice_num(LED_GREEN);

    // Definir wrap para 255 (8 bits)
    pwm_set_wrap(slice_red, 255);
    pwm_set_wrap(slice_green, 255);

    // Ativar os slices PWM
    pwm_set_enabled(slice_red, true);
    pwm_set_enabled(slice_green, true);

    // Inicializar LEDs desligados
    pwm_set_gpio_level(LED_RED, 0);
    pwm_set_gpio_level(LED_GREEN, 0);
}

void reset_leds() {
    pwm_set_gpio_level(LED_RED, 0);
    pwm_set_gpio_level(LED_GREEN, 0);
}

// ========================================================================
// FUNÇÕES DE CONTROLE
// ========================================================================
bool button_pressed() {
    static bool last_state = true;
    bool current_state = gpio_get(SW);

    if (last_state && !current_state) {
        last_state = current_state;
        sleep_ms(50);
        return true;
    }
    last_state = current_state;
    return false;
}

bool button_a_pressed() {
    return gpio_get(BUTTON_A);
}

bool button_b_pressed() {
    return gpio_get(BUTTON_B); 
}

// ========================================================================
// FUNÇÃO DE EXIBIÇÃO DA TABELA VERDADE (CORRIGIDA)
// ========================================================================
void draw_truth_table(bool a, bool b, bool result, const char* gate_name) {
    ssd1306_clear(&disp);
    
    // Título centralizado
    uint8_t titulo_x = (128 - (strlen(gate_name) * 6)) / 2; // Parênteses corrigido
    ssd1306_draw_string(&disp, titulo_x, 2, 1, gate_name);

    // Entradas
    char buffer[30];
    if(strcmp(gate_name, "NOT") == 0) {
        snprintf(buffer, sizeof(buffer), "A: %d", a);
    } else {
        snprintf(buffer, sizeof(buffer), "A: %d  B: %d", a, b);
    }
    ssd1306_draw_string(&disp, 20, 20, 1, buffer);

    // Saída
    snprintf(buffer, sizeof(buffer), "Saida: %d", result);
    ssd1306_draw_string(&disp, 20, 40, 1, buffer);
    
    // Controle dos LEDs
    gpio_put(LED_RED, !result);
    gpio_put(LED_GREEN, result);
    
    // Controle dos LEDs com PWM (50% de brilho)
    if(result) {
        pwm_set_gpio_level(LED_GREEN, 127); // 50% de 255
        pwm_set_gpio_level(LED_RED, 0);
    } else {
        pwm_set_gpio_level(LED_RED, 127);
        pwm_set_gpio_level(LED_GREEN, 0);
    }
    
    ssd1306_show(&disp);
}

// ========================================================================
// FUNÇÕES DAS PORTAS LÓGICAS (TODAS IMPLEMENTADAS)
// ========================================================================
void porta_AND() {
    in_program = true;
    while(true) {
        bool a = button_a_pressed(); // 0 quando pressionado
        bool b = button_b_pressed();
        draw_truth_table(a, b, a && b, "AND");
        if(button_pressed()) break;
        sleep_ms(10);
    }
    reset_leds();
    in_program = false;
}

void porta_OR() {
    in_program = true;
    while(true) {
        bool a = button_a_pressed();
        bool b = button_b_pressed();
        draw_truth_table(a, b, a || b, "OR");
        if(button_pressed()) break;
        sleep_ms(10);
    }
    reset_leds();
    in_program = false;
}

void porta_NOT() {
    in_program = true;
    while(true) {
        bool a = button_a_pressed();
        draw_truth_table(a, false, !a, "NOT"); // Saída invertida
        if(button_pressed()) break;
        sleep_ms(10);
    }
    reset_leds();
    in_program = false;
}

void porta_NAND() {
    in_program = true;
    while(true) {
        bool a = button_a_pressed();
        bool b = button_b_pressed();
        draw_truth_table(a, b, !(a && b), "NAND"); // Saída invertida
        if(button_pressed()) break;
        sleep_ms(10);
    }
    reset_leds();
    in_program = false;
}

void porta_NOR() {
    in_program = true;
    while(true) {
        bool a = button_a_pressed();
        bool b = button_b_pressed();
        draw_truth_table(a, b, !(a || b), "NOR"); // Saída invertida
        if(button_pressed()) break;
        sleep_ms(10);
    }
    reset_leds();
    in_program = false;
}
void porta_XOR() {
    in_program = true;
    while(true) {
        bool a = button_a_pressed();
        bool b = button_b_pressed();
        draw_truth_table(a, b, a ^ b, "XOR");
        if(button_pressed()) break;
        sleep_ms(10);
    }
    reset_leds();
    in_program = false;
}

void porta_XNOR() {
    in_program = true;
    while(true) {
        bool a = button_a_pressed();
        bool b = button_b_pressed();
        draw_truth_table(a, b, !(a ^ b), "XNOR");
        if(button_pressed()) break;
        sleep_ms(10);
    }
    reset_leds();
    in_program = false;
}

// ========================================================================
// CONTROLE DO MENU 
// ========================================================================

void handle_joystick() {
    adc_select_input(ADC_CHANNEL_Y);
    int y = adc_read();
    
    const int CENTER = 2048;
    const int THRESHOLD = 300;
    const uint32_t DEBOUNCE_TIME_MS = 200;

    if(absolute_time_diff_us(menu.last_move, get_absolute_time()) < DEBOUNCE_TIME_MS * 1000) {
        return;
    }

    // Lógica invertida para corrigir a direção
    if(y < (CENTER - THRESHOLD)) { // Movimento para BAIXO
        menu.index = (menu.index + 1) % MENU_ITEMS;
        menu.last_move = get_absolute_time();
    }
    else if(y > (CENTER + THRESHOLD)) { // Movimento para CIMA
        menu.index = (menu.index > 0) ? menu.index - 1 : MENU_ITEMS - 1;
        menu.last_move = get_absolute_time();
    }

    update_display();
}

// ========================================================================
// FUNÇÕES AUXILIARES
// ========================================================================
void handle_button() {
    if(button_pressed() && !in_program) {
        switch(menu.index) {
            case 0: porta_AND();  break;
            case 1: porta_OR();   break;
            case 2: porta_NOT();  break;
            case 3: porta_NAND(); break;
            case 4: porta_NOR();  break;
            case 5: porta_XOR();  break;
            case 6: porta_XNOR(); break;
        }
        update_display();
    }
}

// ========================================================================
// LOOP PRINCIPAL
// ========================================================================
int main() {
    inicializa_hardware();
    update_display();

    while(true) {
        if(!in_program) {
            handle_joystick();
            handle_button();
            if(absolute_time_diff_us(menu.last_move, get_absolute_time()) > 1000000) {
                update_display();
                menu.last_move = get_absolute_time();
            }
        }
        sleep_ms(10);
    }
    return 0;
}