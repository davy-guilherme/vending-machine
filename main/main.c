#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "st7735.h"
#include "a4988.h"

// ==========================================
// CONFIGURAÇÕES E PINOS (Fácil manutenção)
// ==========================================
#define PIN_BTN_ESQ     19
#define PIN_BTN_DIR     21
#define PIN_BALANCA     23

#define PIN_DISP_SCK    18
#define PIN_DISP_MOSI   22 // 18 e 19 ou 22 e 23
#define PIN_DISP_CS     5 
#define PIN_DISP_DC     2 // A0? o que faz este pino?
#define PIN_DISP_RST    4

// Pinos dos Motores corrigidos (sem sobreposição)
#define PIN_M1_STEP     26
#define PIN_M1_DIR      25
#define PIN_M1_EN       27

#define PIN_M2_STEP     14
#define PIN_M2_DIR      12 
#define PIN_M2_EN       13 

#define DEBOUNCE_MS     250
// #define TOTAL_PRODUTOS  3
#define TIMEOUT_MOTOR   10000 // 10 segundos máximo para o motor girar

// ==========================================
// ESTRUTURAS DE DADOS
// ==========================================
typedef enum {
    ESTADO_ESCOLHER_PRODUTO,
    ESTADO_DISPENSANDO,
    ESTADO_LIBERANDO_RETIRADA,
    ESTADO_AGRADECER,
    ESTADO_ERRO_DISPENSA
} EstadoMaquina;

typedef enum {
    EVT_NENHUM,
    EVT_BOTAO_ESQ,
    EVT_BOTAO_DIR
} EventoBotao;

typedef struct {
    uint8_t id;
    char nome[20];            // Nome ajustável
    uint16_t preco_centavos;
    uint8_t estoque;
    uint8_t motor_id;         // Qual motor (1 ou 2) atua neste produto
} Produto;

typedef struct {
    Produto *produtos;        // Array dinâmico na RAM
    uint8_t total_produtos;
    uint8_t selecionado;
} Menu;

// ==========================================
// VARIÁVEIS GLOBAIS
// ==========================================
st7735_t display;
a4988_t motor_p1;
a4988_t motor_p2;

EstadoMaquina estado_atual = ESTADO_ESCOLHER_PRODUTO;
uint32_t tempo_estado_atual = 0; // Guarda o "millis" de quando o estado iniciou
bool atualizar_tela = true;      // Flag para evitar redesenhar a tela sem necessidade

// 1. Array com os produtos iniciais cadastrados
static Produto produtos_iniciais[] = {
    {.id = 1, .nome = "Castanhas",  .preco_centavos = 700, .estoque = 5, .motor_id = 1},
    {.id = 2, .nome = "Chocolate",  .preco_centavos = 500, .estoque = 3, .motor_id = 2},
    {.id = 3, .nome = "Salgadinho", .preco_centavos = 420, .estoque = 0, .motor_id = 1},
};

// Estrutura Menu apontando para o array inicial
static Menu meu_menu = {
    .produtos = produtos_iniciais,
    .total_produtos = sizeof(produtos_iniciais) / sizeof(produtos_iniciais[0]), // Calcula automaticamente
    .selecionado = 0
};

// ==========================================
// FUNÇÕES AUXILIARES
// ==========================================
uint32_t millis(void) {
    return (uint32_t)(esp_timer_get_time() / 1000);
}

// ==========================================
// FUNÇÕES DE INTERFACE (DISPLAY)
// ==========================================
void desenhar_rodape_botoes(st7735_t *dev, const char *txt_esq, const char *txt_dir) {
    uint16_t y_rodape = 140;
    st7735_fill_rect(dev, 0, y_rodape, 63, 20, COLOR_MOSS_GREEN);
    st7735_draw_string(dev, 4, y_rodape + 6, txt_esq, COLOR_CREAM);
    st7735_fill_rect(dev, 63, y_rodape, 2, 20, COLOR_SAGE_DARK);
    st7735_fill_rect(dev, 65, y_rodape, 63, 20, COLOR_ORANGE_WARM);
    st7735_draw_string(dev, 69, y_rodape + 6, txt_dir, COLOR_WHITE);
}

void desenhar_tela(st7735_t *dev, EstadoMaquina estado, Menu *m) {
    st7735_fill_screen(dev, COLOR_SAGE_DARK); // Limpa o fundo

    if (estado == ESTADO_ESCOLHER_PRODUTO) {
        st7735_fill_rect(dev, 0, 0, 128, 20, COLOR_MOSS_GREEN);
        st7735_draw_string(dev, 10, 5, "SELECIONE:", COLOR_CREAM);

        uint16_t y_inicial = 35;
        for (uint8_t i = 0; i < m->total_produtos; i++) {
            uint16_t y = y_inicial + (i * 25);
            if (i == m->selecionado) {
                st7735_fill_rect(dev, 2, y - 3, 124, 18, COLOR_ORANGE_WARM);
                st7735_draw_string(dev, 5, y, ">", COLOR_AMBER);
                st7735_draw_string(dev, 15, y, m->produtos[i].nome, COLOR_WHITE);
            } else {
                st7735_draw_string(dev, 15, y, m->produtos[i].nome, COLOR_CREAM);
            }

            if (m->produtos[i].estoque == 0) {
                st7735_draw_string(dev, 95, y, "ESG", COLOR_RED);
            }
        }
        desenhar_rodape_botoes(dev, "MUDAR", "ENTRA");
    } 
    else if (estado == ESTADO_DISPENSANDO) {
        st7735_fill_rect(dev, 0, 0, 128, 20, COLOR_ORANGE_WARM);
        st7735_draw_string(dev, 10, 5, "PROCESSANDO", COLOR_WHITE);
        st7735_draw_string(dev, 10, 35, "Entregando...", COLOR_CREAM);
        st7735_draw_string(dev, 10, 55, m->produtos[m->selecionado].nome, COLOR_AMBER);
    }
    else if (estado == ESTADO_LIBERANDO_RETIRADA) {
        st7735_fill_rect(dev, 0, 0, 128, 20, COLOR_GREEN);
        st7735_draw_string(dev, 10, 5, "PRONTINHO!", COLOR_WHITE);
        st7735_draw_string(dev, 10, 45, "Pode retirar", COLOR_CREAM);
        st7735_draw_string(dev, 10, 65, "seu produto.", COLOR_WHITE);
    }
    else if (estado == ESTADO_AGRADECER) {
        st7735_draw_string(dev, 20, 50, "MUITO", COLOR_WHITE);
        st7735_draw_string(dev, 20, 70, "OBRIGADO!", COLOR_AMBER);
    }
    else if (estado == ESTADO_ERRO_DISPENSA) {
        st7735_fill_rect(dev, 0, 0, 128, 20, COLOR_RED);
        st7735_draw_string(dev, 10, 5, "FALHA!", COLOR_WHITE);
        st7735_draw_string(dev, 10, 45, "Produto preso", COLOR_CREAM);
        st7735_draw_string(dev, 10, 65, "Consulte suporte.", COLOR_WHITE);
    }
}

// ==========================================
// LEITURA DE BOTÕES (Não bloqueante)
// ==========================================
EventoBotao ler_botoes() {
    static uint32_t ultimo_debounce_esq = 0;
    static uint32_t ultimo_debounce_dir = 0;
    uint32_t agora = millis();

    if (gpio_get_level(PIN_BTN_ESQ) == 0 && (agora - ultimo_debounce_esq > DEBOUNCE_MS)) {
        ultimo_debounce_esq = agora;
        return EVT_BOTAO_ESQ;
    }
    if (gpio_get_level(PIN_BTN_DIR) == 0 && (agora - ultimo_debounce_dir > DEBOUNCE_MS)) {
        ultimo_debounce_dir = agora;
        return EVT_BOTAO_DIR;
    }
    return EVT_NENHUM;
}

// ==========================================
// FUNÇÃO PRINCIPAL (Super Loop)
// ==========================================
void app_main(void) {
    // 1. Configuração de Hardware
    gpio_set_direction(PIN_BTN_DIR, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_BTN_ESQ, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_BTN_DIR, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_BTN_ESQ, GPIO_PULLUP_ONLY);

    gpio_set_direction(PIN_BALANCA, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_BALANCA, GPIO_PULLUP_ONLY);

    st7735_init(&display, SPI2_HOST, PIN_DISP_SCK, PIN_DISP_MOSI, PIN_DISP_CS, PIN_DISP_DC, PIN_DISP_RST);
    a4988_init(&motor_p1, PIN_M1_STEP, PIN_M1_DIR, PIN_M1_EN, 13000);
    a4988_init(&motor_p2, PIN_M2_STEP, PIN_M2_DIR, PIN_M2_EN, 13000);

    // Splash inicial (só roda 1 vez, pode usar delay aqui)
    st7735_fill_screen(&display, COLOR_BLACK);
    st7735_draw_string(&display, 10, 30, "Iniciando...", COLOR_WHITE);
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 2. Loop Principal (Máquina de Estados Não-Bloqueante)
    while (1) {
        uint32_t agora = millis();
        EventoBotao evt = ler_botoes(); // Lê os botões a cada ciclo
        
        // Se a flag de redesenhar a tela estiver ativa, desenha apenas uma vez
        if (atualizar_tela) {
            desenhar_tela(&display, estado_atual, &meu_menu);
            atualizar_tela = false; 
        }

        switch (estado_atual) {
            case ESTADO_ESCOLHER_PRODUTO:
                if (evt == EVT_BOTAO_ESQ) {
                    // Alterna opção
                    meu_menu.selecionado = (meu_menu.selecionado + 1) % meu_menu.total_produtos;
                    atualizar_tela = true;
                } 
                else if (evt == EVT_BOTAO_DIR) {
                    // Confirma seleção
                    if (meu_menu.produtos[meu_menu.selecionado].estoque > 0) {
                        estado_atual = ESTADO_DISPENSANDO;
                        tempo_estado_atual = agora; // Marca a hora que iniciou a dispensa
                        atualizar_tela = true;
                    }
                }
                break;

            case ESTADO_DISPENSANDO:
                // Motor gira em pequenos passos a cada volta do loop (NÃO BLOQUEIA O ESP32)
                if (meu_menu.selecionado == 0 || meu_menu.selecionado == 2) {
                    a4988_step(&motor_p1, 6, A4988_DIR_HOR); // Dá 10 passinhos rápidos e solta a CPU
                } else {
                    a4988_step(&motor_p2, 6, A4988_DIR_HOR);
                }

                // Condição de Sucesso: Produto caiu na balança
                if (gpio_get_level(PIN_BALANCA) == 0) {
                    a4988_disable(&motor_p1);
                    a4988_disable(&motor_p2);
                    meu_menu.produtos[meu_menu.selecionado].estoque--; // Debita estoque apenas se entregou
                    estado_atual = ESTADO_LIBERANDO_RETIRADA;
                    tempo_estado_atual = agora;
                    atualizar_tela = true;
                }
                // Condição de Falha: Produto engasgou (Timeout)
                else if (agora - tempo_estado_atual > TIMEOUT_MOTOR) {
                    a4988_disable(&motor_p1);
                    a4988_disable(&motor_p2);
                    estado_atual = ESTADO_ERRO_DISPENSA;
                    tempo_estado_atual = agora;
                    atualizar_tela = true;
                }
                break;

            case ESTADO_LIBERANDO_RETIRADA:
                // Espera 4 segundos SEM TRAVAR o processador
                if (agora - tempo_estado_atual > 4000) {
                    estado_atual = ESTADO_AGRADECER;
                    tempo_estado_atual = agora;
                    atualizar_tela = true;
                }
                break;

            case ESTADO_AGRADECER:
                // Espera 3 segundos SEM TRAVAR o processador
                if (agora - tempo_estado_atual > 3000) {
                    estado_atual = ESTADO_ESCOLHER_PRODUTO;
                    atualizar_tela = true;
                }
                break;

            case ESTADO_ERRO_DISPENSA:
                // Mostra erro por 4 segundos e volta para o menu
                if (agora - tempo_estado_atual > 4000) {
                    estado_atual = ESTADO_ESCOLHER_PRODUTO;
                    atualizar_tela = true;
                }
                break;
        }

        // ÚNICO DELAY DO LOOP INTEIRO: Fundamental para o FreeRTOS respirar (Idle Task)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}