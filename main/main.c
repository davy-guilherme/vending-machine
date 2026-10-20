#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "st7735.h"
#include "a4988.h"

// Instâncias de Hardware
st7735_t display;
a4988_t motor;

#define btn_esq 19
#define btn_dir 21
#define DEBOUNCE_MS 100

uint32_t ultimoDebounce_esq = 0;
uint32_t ultimoDebounce_dir = 0;

// Estrutura de Dados do Menu
typedef struct {
    const char *opcoes[3];
    uint8_t total_opcoes;
    uint8_t selecionado;
} Menu;

static Menu meu_menu = {
    .opcoes = {"1. Produto 1", "2. Produto 2", "3. Produto 3"},
    .total_opcoes = 3,
    .selecionado = 0
};

/**
 * Desenha o menu na tela ST7735 aplicando destaque na opção selecionada
 */
void desenhar_menu(st7735_t *dev, Menu *m) {
    // Limpa a tela
    st7735_fill_screen(dev, COLOR_BLACK);

    // Cabeçalho / Título
    st7735_fill_rect(dev, 0, 0, 128, 20, COLOR_BLUE);
    st7735_draw_string(dev, 10, 5, "MENU OPCOES", COLOR_WHITE);

    // Desenha as Opções do Menu
    uint16_t y_inicial = 35;
    uint8_t espacamento_y = 20;

    for (uint8_t i = 0; i < m->total_opcoes; i++) {
        uint16_t y_pos = y_inicial + (i * espacamento_y);

        if (i == m->selecionado) {
            // Opção Selecionada: Fundo cinza com seta e texto em amarelo
            st7735_fill_rect(dev, 5, y_pos - 3, 118, 16, COLOR_GRAY);
            st7735_draw_string(dev, 10, y_pos, ">", COLOR_YELLOW);
            st7735_draw_string(dev, 22, y_pos, m->opcoes[i], COLOR_YELLOW);
        } else {
            // Opção Normal: Sem fundo, texto em branco
            st7735_draw_string(dev, 22, y_pos, m->opcoes[i], COLOR_WHITE);
        }
    }
}

/**
 * Executa a ação da opção selecionada no menu
 */
void executar_acao_menu(uint8_t opcao) {
    switch (opcao) {
        case 0:
            printf("Produto 1 Selecionad0\n");
            break;

        case 1:
            printf("Produto 2 Selecionado\n");
            // a4988_step(&motor, 600, A4988_DIR_HOR);
            // a4988_disable(&motor);
            break;

        case 2:
            printf("Produto Selecionado\n");
            // a4988_step(&motor, 300, A4988_DIR_ANT);
            // a4988_disable(&motor);
            break;

        default:
            break;
    }
}

void configure_btns () {
    gpio_reset_pin(btn_dir);
    gpio_reset_pin(btn_esq);

    gpio_set_direction(btn_dir, GPIO_MODE_INPUT);
    gpio_set_direction(btn_esq, GPIO_MODE_INPUT);

    gpio_set_pull_mode(btn_dir, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(btn_esq, GPIO_PULLUP_ONLY);
}

uint32_t millis(void) {
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void app_main(void) {
    // Inicialização dos botões
    configure_btns();

    // Inicialização do Display (SCK=18, MOSI=22, CS=5, DC=2, RST=4)
    st7735_init(&display, SPI2_HOST, GPIO_NUM_18, GPIO_NUM_22, GPIO_NUM_5, GPIO_NUM_2, GPIO_NUM_4);

    // Inicialização do Motor A4988 (STEP=26, DIR=25, ENABLE=27, Delay=2000 us)
    a4988_init(&motor, GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_27, 2000);

    // Tela de Splash Inicial
    st7735_fill_screen(&display, COLOR_BLACK);
    st7735_fill_rect(&display, 5, 5, 118, 20, COLOR_BLUE);
    st7735_draw_string(&display, 10, 10, "Bem-vindo", COLOR_WHITE);
    st7735_draw_string(&display, 10, 35, "Maquina", COLOR_GREEN);
    st7735_draw_string(&display, 10, 60, "de Vendas", COLOR_RED);

    vTaskDelay(pdMS_TO_TICKS(3000));

    desenhar_menu(&display, &meu_menu);

    // Loop do Menu com transição automática
    while (1) {
        uint32_t agora = millis();

        // Avança ciclicamente para a próxima opção (0 -> 1 -> 2 -> 0)
        // meu_menu.selecionado = (meu_menu.selecionado + 1) % meu_menu.total_opcoes;

        if (gpio_get_level(btn_dir) == 0) {
            if (agora - ultimoDebounce_dir >= DEBOUNCE_MS) {
                ultimoDebounce_dir = agora; // Atualiza o tempo do debounce
                printf("Botão direito pressonado\n");
                // Avança ciclicamente para a próxima opção (0 -> 1 -> 2 -> 0)
                meu_menu.selecionado = (meu_menu.selecionado + 1) % meu_menu.total_opcoes;
                printf("Menu selecionado: %d\n", meu_menu.selecionado);

                // Executa a ação atribuída à opção ativa
                executar_acao_menu(meu_menu.selecionado);

                // Redesenha a tela destacando a opção atual
                desenhar_menu(&display, &meu_menu);
            }
        }

        if (gpio_get_level(btn_esq) == 0) {
            if (agora - ultimoDebounce_esq >= DEBOUNCE_MS) {
                ultimoDebounce_esq = agora; // Atualiza o tempo do debounce
                printf("Botão esquerdo pressonado\n");
                // Avança ciclicamente para a próxima opção (0 -> 1 -> 2 -> 0)
                meu_menu.selecionado = (meu_menu.selecionado + meu_menu.total_opcoes - 1) % meu_menu.total_opcoes;
                printf("Menu selecionado: %d\n", meu_menu.selecionado);

                // Executa a ação atribuída à opção ativa
                executar_acao_menu(meu_menu.selecionado);

                // Redesenha a tela destacando a opção atual
                desenhar_menu(&display, &meu_menu);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}