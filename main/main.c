#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "st7735.h"
#include "a4988.h"

// Instâncias de Hardware
st7735_t display;
a4988_t motor_p1;
a4988_t motor_p2;

#define btn_esq 19 // alterar
#define btn_dir 21 // selecionar
#define balanca 23
#define DEBOUNCE_MS 500
#define TOTAL_PRODUTOS 4


uint32_t ultimoDebounce_esq = 0;
uint32_t ultimoDebounce_dir = 0;

// Estados do Sistema (Telas)
typedef enum {
    ESCOLHER_PRODUTO,
    CONFIRMAR_PRODUTO,
    // COBRANDO,
    DISPENSANDO_PRODUTO,
    LIBERANDO_RETIRADA,
    AGRADECER,
    // ERRO
} Estado;

static Estado estado_atual = ESCOLHER_PRODUTO;

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
        {.id = 1, .nome = "1. Barra de chocolate", .preco_centavos = 700, .estoque = 5},
        {.id = 2, .nome = "2. Confetes de amendoin 2", .preco_centavos = 500, .estoque = 0},
        {.id = 3, .nome = "3. Produto 3", .preco_centavos = 420, .estoque = 2},
        {.id = 3, .nome = "3. Produto 4", .preco_centavos = 600, .estoque = 8},
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
    desenhar_rodape_botoes(dev, " -- ", " -- ");
}

/**
 * Tela exibida ao liberar o produto para ser retirado
 */
void desenhar_retirar_produto(st7735_t *dev, uint8_t opcao) {
    // 1. Fundo suave em Verde Sálvia Escuro
    st7735_fill_screen(dev, COLOR_SAGE_DARK);

    // 2. Cabeçalho de destaque em Laranja Quente com texto em Branco
    st7735_fill_rect(dev, 0, 0, 128, 20, COLOR_ORANGE_WARM);
    st7735_draw_string(dev, 10, 5, "Prontinho", COLOR_WHITE);

    // 3. Rótulo "Entregando:" em tom Bege/Creme
    st7735_draw_string(dev, 10, 35, "Retire o ", COLOR_CREAM);

    // 4. Nome do produto em destaque em tom Amarelo/Âmbar
    st7735_draw_string(dev, 10, 55, meu_menu.produtos[opcao].nome, COLOR_AMBER);

    // 5. Preço ou status secundário centralizado (Opcional, preenche o espaço visual)
    // st7735_draw_string(dev, 10, 85, "Aguarde...", COLOR_CREAM);

    // 6. Barra inferior dividida (Esquerda: sem ação / Direita: Voltar)
    desenhar_rodape_botoes(dev, " -- ", " -- ");
}

/**
 * Executa a ação da opção selecionada no menu
 */
void executar_acao_menu(uint8_t opcao) {
    switch (opcao) {
        case 0:
            estado_atual = DISPENSANDO_PRODUTO;
            printf("Produto 1 Selecionad0\n");
            while (gpio_get_level(balanca) == 1) {
                printf("%d\n", gpio_get_level(balanca));
                a4988_step(&motor_p1, 4, A4988_DIR_HOR);
            }
            a4988_disable(&motor_p1);
            estado_atual = LIBERANDO_RETIRADA;
            break;

        case 1:
            printf("Produto 2 Selecionado\n");
            estado_atual = DISPENSANDO_PRODUTO;
            while (gpio_get_level(balanca) == 1) {
                a4988_step(&motor_p2, 600, A4988_DIR_HOR);
            }
            a4988_disable(&motor_p2);
            estado_atual = LIBERANDO_RETIRADA;
            break;

        case 2:
            printf("Produto Selecionado\n");
            estado_atual = DISPENSANDO_PRODUTO;
            while (gpio_get_level(balanca) == 1) {
                a4988_step(&motor_p1, 600, A4988_DIR_HOR);
            }
            a4988_disable(&motor_p1);
            break;

        default:
            break;
    }
}

/**
 * Executa a ação do motor_p1 associada ao produto
 */
void executar_acao_produto(uint8_t opcao) {
    switch (opcao) {
        case 0:
            printf("Executando dispensa do Produto 1...\n");
            // ex: a4988_step(&motor_p1, 200, A4988_DIR_HOR);
            break;

        case 1:
            printf("Executando dispensa do Produto 2...\n");
            // a4988_step(&motor_p1, 600, A4988_DIR_HOR);
            // a4988_disable(&motor_p1);
            break;

        case 2:
            printf("Executando dispensa do Produto 3...\n");
            // a4988_step(&motor_p1, 300, A4988_DIR_ANT);
            // a4988_disable(&motor_p1);
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

void configure_balanca () {
    gpio_reset_pin(balanca);
    gpio_set_direction(balanca, GPIO_MODE_INPUT);
    gpio_set_pull_mode(balanca, GPIO_PULLUP_ONLY);
}

uint32_t millis(void) {
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void app_main(void) {
    // Inicialização dos botões
    configure_btns();

    // Inicialiar balanca
    configure_balanca();

    // Inicialização do Display (SCK=18, MOSI=22, CS=5, DC=2, RST=4)
    st7735_init(&display, SPI2_HOST, GPIO_NUM_18, GPIO_NUM_22, GPIO_NUM_5, GPIO_NUM_2, GPIO_NUM_4);

    // Inicialização do motor A4988 (STEP=26, DIR=25, ENABLE=27, Delay=2000 us)
    a4988_init(&motor_p1, GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_27, 3000);
    a4988_init(&motor_p2, GPIO_NUM_14, GPIO_NUM_27, GPIO_NUM_26, 3000);

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

        if (estado_atual == LIBERANDO_RETIRADA) {
            printf("Liberando Retirada");
            // mostrar tela solicitando que o usuário pegue o produto
            desenhar_retirar_produto(&display, meu_menu.selecionado);
            
            // aguardar 4 segundos
            vTaskDelay(pdMS_TO_TICKS(4000));

            // alterar estado
            estado_atual = AGRADECER;
        }

        if (estado_atual == AGRADECER) {
            // mostrar tela agradecendo

            // aguardar 3 segundos

            // voltar para o primeiro estado 
            estado_atual = ESCOLHER_PRODUTO;
            desenhar_menu(&display, &meu_menu);
        }
        // ------------------ BOTÃO ESQUERDO (Alterna entre opções) ------------------
        if (gpio_get_level(btn_esq) == 0) {
            if (agora - ultimoDebounce_esq >= DEBOUNCE_MS) {
                ultimoDebounce_esq = agora; // Atualiza o tempo do debounce
                printf("Botão esquerdo pressionado\n");

                if (estado_atual == ESCOLHER_PRODUTO) {
                    // Avança ciclicamente para a próxima opção (0 -> 1 -> 2 -> 0)
                    meu_menu.selecionado = (meu_menu.selecionado + 1) % TOTAL_PRODUTOS;
                    printf("Menu selecionado: %d\n", meu_menu.selecionado);

                    // Executa a ação atribuída à opção ativa
                    // executar_acao_menu(meu_menu.selecionado);

                    // Redesenha a tela destacando a opção atual
                    desenhar_menu(&display, &meu_menu);
                } else if (estado_atual == DISPENSANDO_PRODUTO) {
                    // NADA
                }
            }
            
        }

        // ------------------ BOTÃO DIREITO (Seleciona / Entra na Tela) ------------------
        if (gpio_get_level(btn_dir) == 0) {
            if (agora - ultimoDebounce_dir >= DEBOUNCE_MS) {
                ultimoDebounce_dir = agora;
                printf("Botão direito pressionado\n");

                if (estado_atual == ESCOLHER_PRODUTO) {
                    Produto *p_selecionado = &meu_menu.produtos[meu_menu.selecionado];

                    if (p_selecionado->estoque > 0) {
                        // Tem estoque: decrementa e inicia a entrega
                        p_selecionado->estoque--; 
                        
                        estado_atual = DISPENSANDO_PRODUTO;
                        desenhar_tela_dispensando(&display, meu_menu.selecionado);
                        // executar_acao_produto(meu_menu.selecionado);
                        executar_acao_menu(meu_menu.selecionado);
                    } else {
                        // Sem estoque: exibe aviso / ignora o clique
                        printf("Produto esgotado! Estoque: 0\n");
                        // Opcional: desenhar mensagem rápida "Esgotado!" na tela
                    }
                } else if (estado_atual == DISPENSANDO_PRODUTO) {
                    // nada
                    // estado_atual = ESCOLHER_PRODUTO;
                    // desenhar_menu(&display, &meu_menu);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}