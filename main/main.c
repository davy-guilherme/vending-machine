#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "st7735.h"
#include "a4988.h"

// Instâncias de Hardware
st7735_t display;
a4988_t motor;

#define btn_esq 19 // alterar
#define btn_dir 21 // selecionar
#define DEBOUNCE_MS 500
#define TOTAL_PRODUTOS 3


uint32_t ultimoDebounce_esq = 0;
uint32_t ultimoDebounce_dir = 0;

// Estados do Sistema (Telas)
typedef enum {
    MENU_PRINCIPAL,
    // CONFIRMANDO_PRODUTO,
    // COBRANDO,
    DISPENSANDO_PRODUTO,
    // LIBERANDO_RETIRADA,
    // AGRADECENDO,
    // ERRO
} Estado;

static Estado tela_atual = MENU_PRINCIPAL;

// Estrutura de Dados dos Produtos
typedef struct {
    uint8_t id;
    const char *nome;
    uint16_t preco_centavos; // Exemplo 500 para 5,00
    uint8_t estoque;
} Produto;

// Estrutura de Dados do Menu
typedef struct {
    Produto produtos[TOTAL_PRODUTOS];
    // const char *opcoes[3];
    // uint8_t total_opcoes;
    uint8_t selecionado;
} Menu;

static Menu meu_menu = {
    .produtos = {
        {.id = 1, .nome = "1. Produto 1", .preco_centavos = 350, .estoque = 5},
        {.id = 2, .nome = "2. Produto 2", .preco_centavos = 500, .estoque = 0},
        {.id = 3, .nome = "3. Produto 3", .preco_centavos = 400, .estoque = 2},
    },
    // .opcoes = {"1. Produto 1", "2. Produto 2", "3. Produto 3"},
    // .total_opcoes = 3,
    .selecionado = 0
};

/**
 * Desenha a barra inferior dividida em duas caixas contextuais
 */
void desenhar_rodape_botoes(st7735_t *dev, const char *txt_esq, const char *txt_dir) {
    uint16_t y_rodape = 140;
    uint16_t alt_rodape = 20;

    // --- LADO ESQUERDO (Botão Esquerdo) ---
    // Fundo em Verde Musgo para indicar área clicável
    st7735_fill_rect(dev, 0, y_rodape, 63, alt_rodape, COLOR_MOSS_GREEN);
    // Texto em tom Creme (alinhado à esquerda)
    st7735_draw_string(dev, 4, y_rodape + 6, txt_esq, COLOR_CREAM);

    // Linha divisória vertical no centro (separador)
    st7735_fill_rect(dev, 63, y_rodape, 2, alt_rodape, COLOR_SAGE_DARK);

    // --- LADO DIREITO (Botão Direito) ---
    // Fundo em Laranja Quente (destaque para ação/seleção)
    st7735_fill_rect(dev, 65, y_rodape, 63, alt_rodape, COLOR_ORANGE_WARM);
    // Texto em Branco
    st7735_draw_string(dev, 69, y_rodape + 6, txt_dir, COLOR_WHITE);
}

/**
 * Desenha o menu na tela ST7735 aplicando destaque na opção selecionada
 */
void desenhar_menu(st7735_t *dev, Menu *m) {
    // 1. Fundo
    st7735_fill_screen(dev, COLOR_SAGE_DARK);

    // 2. Cabeçalho
    st7735_fill_rect(dev, 0, 0, 128, 20, COLOR_MOSS_GREEN);
    st7735_draw_string(dev, 10, 5, "SELECIONE:", COLOR_CREAM);

    uint16_t y_inicial = 35;
    uint8_t espacamento_y = 25;

    for (uint8_t i = 0; i < TOTAL_PRODUTOS; i++) {
        uint16_t y_pos = y_inicial + (i * espacamento_y);
        Produto *p = &m->produtos[i];

        if (i == m->selecionado) {
            // Opção Selecionada: Retângulo Laranja Quente com texto Branco
            st7735_fill_rect(dev, 2, y_pos - 3, 124, 18, COLOR_ORANGE_WARM);
            st7735_draw_string(dev, 5, y_pos, ">", COLOR_AMBER);
            st7735_draw_string(dev, 15, y_pos, p->nome, COLOR_WHITE);
        } else {
            // Opção Normal: Texto em tom Bege/Creme para leitura suave
            st7735_draw_string(dev, 15, y_pos, p->nome, COLOR_CREAM);
        }

        // Exibe indicação de Esgotado
        if (p->estoque == 0) {
            st7735_draw_string(dev, 95, y_pos, "ESG", COLOR_RED);
        }
    }

    // Desenha o rodapé duplo no final
    desenhar_rodape_botoes(dev, "MUDAR", "ENTRA");
}

/**
 * Tela exibida ao confirmar a seleção de um produto
 */
void desenhar_tela_dispensando(st7735_t *dev, uint8_t opcao) {
    // 1. Fundo suave em Verde Sálvia Escuro
    st7735_fill_screen(dev, COLOR_SAGE_DARK);

    // 2. Cabeçalho de destaque em Laranja Quente com texto em Branco
    st7735_fill_rect(dev, 0, 0, 128, 20, COLOR_ORANGE_WARM);
    st7735_draw_string(dev, 10, 5, "PROCESSANDO", COLOR_WHITE);

    // 3. Rótulo "Entregando:" em tom Bege/Creme
    st7735_draw_string(dev, 10, 35, "Entregando", COLOR_CREAM);

    // 4. Nome do produto em destaque em tom Amarelo/Âmbar
    st7735_draw_string(dev, 10, 55, meu_menu.produtos[opcao].nome, COLOR_AMBER);

    // 5. Preço ou status secundário centralizado (Opcional, preenche o espaço visual)
    st7735_draw_string(dev, 10, 85, "Aguarde...", COLOR_CREAM);

    // 6. Barra inferior dividida (Esquerda: sem ação / Direita: Voltar)
    desenhar_rodape_botoes(dev, "  --", "VOLTAR>");
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

/**
 * Executa a ação do motor associada ao produto
 */
void executar_acao_produto(uint8_t opcao) {
    switch (opcao) {
        case 0:
            printf("Executando dispensa do Produto 1...\n");
            // ex: a4988_step(&motor, 200, A4988_DIR_HOR);
            break;

        case 1:
            printf("Executando dispensa do Produto 2...\n");
            // a4988_step(&motor, 600, A4988_DIR_HOR);
            // a4988_disable(&motor);
            break;

        case 2:
            printf("Executando dispensa do Produto 3...\n");
            // a4988_step(&motor, 300, A4988_DIR_ANT);
            // a4988_disable(&motor);
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

        if (gpio_get_level(btn_esq) == 0) {
            if (agora - ultimoDebounce_esq >= DEBOUNCE_MS) {
                ultimoDebounce_esq = agora; // Atualiza o tempo do debounce
                printf("Botão esquerdo pressonado\n");
                // Avança ciclicamente para a próxima opção (0 -> 1 -> 2 -> 0)
                meu_menu.selecionado = (meu_menu.selecionado + 1) % TOTAL_PRODUTOS;
                printf("Menu selecionado: %d\n", meu_menu.selecionado);

                // Executa a ação atribuída à opção ativa
                executar_acao_menu(meu_menu.selecionado);

                // Redesenha a tela destacando a opção atual
                desenhar_menu(&display, &meu_menu);
            }
        }

        // ------------------ BOTÃO DIREITO (Seleciona / Entra na Tela) ------------------
        if (gpio_get_level(btn_dir) == 0) {
            if (agora - ultimoDebounce_dir >= DEBOUNCE_MS) {
                ultimoDebounce_dir = agora;

                if (tela_atual == MENU_PRINCIPAL) {
                    Produto *p_selecionado = &meu_menu.produtos[meu_menu.selecionado];

                    if (p_selecionado->estoque > 0) {
                        // Tem estoque: decrementa e inicia a entrega
                        p_selecionado->estoque--; 
                        
                        tela_atual = DISPENSANDO_PRODUTO;
                        desenhar_tela_dispensando(&display, meu_menu.selecionado);
                        executar_acao_produto(meu_menu.selecionado);
                    } else {
                        // Sem estoque: exibe aviso / ignora o clique
                        printf("Produto esgotado! Estoque: 0\n");
                        // Opcional: desenhar mensagem rápida "Esgotado!" na tela
                    }
                } else if (tela_atual == DISPENSANDO_PRODUTO) {
                    tela_atual = MENU_PRINCIPAL;
                    desenhar_menu(&display, &meu_menu);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}