#include "st7735.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Fontes 5x7 das letras A-Z
static const uint8_t fonte_A[7] = {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
static const uint8_t fonte_B[7] = {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110};
static const uint8_t fonte_C[7] = {0b01111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b01111};
static const uint8_t fonte_D[7] = {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110};
static const uint8_t fonte_E[7] = {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
static const uint8_t fonte_F[7] = {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000};
static const uint8_t fonte_G[7] = {0b01111, 0b10000, 0b10000, 0b10011, 0b10001, 0b10001, 0b01110};
static const uint8_t fonte_H[7] = {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
static const uint8_t fonte_I[7] = {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
static const uint8_t fonte_J[7] = {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100};
static const uint8_t fonte_K[7] = {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001};
static const uint8_t fonte_L[7] = {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111};
static const uint8_t fonte_M[7] = {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001};
static const uint8_t fonte_N[7] = {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001};
static const uint8_t fonte_O[7] = {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
static const uint8_t fonte_P[7] = {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000};
static const uint8_t fonte_Q[7] = {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101};
static const uint8_t fonte_R[7] = {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001};
static const uint8_t fonte_S[7] = {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};
static const uint8_t fonte_T[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
static const uint8_t fonte_U[7] = {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
static const uint8_t fonte_V[7] = {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100};
static const uint8_t fonte_W[7] = {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001};
static const uint8_t fonte_X[7] = {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001};
static const uint8_t fonte_Y[7] = {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100};
static const uint8_t fonte_Z[7] = {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111};

// Fontes 5x7 das letras minúsculas a-z
static const uint8_t fonte_a_min[7] = {0b00000, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111};
static const uint8_t fonte_b_min[7] = {0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b11001, 0b10110};
static const uint8_t fonte_c_min[7] = {0b00000, 0b00000, 0b01110, 0b10000, 0b10000, 0b10000, 0b01110};
static const uint8_t fonte_d_min[7] = {0b00001, 0b00001, 0b00111, 0b10001, 0b10001, 0b10001, 0b01111};
static const uint8_t fonte_e_min[7] = {0b00000, 0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b01110};
static const uint8_t fonte_f_min[7] = {0b00110, 0b01001, 0b01000, 0b11100, 0b01000, 0b01000, 0b01000};
static const uint8_t fonte_g_min[7] = {0b00000, 0b01111, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110};
static const uint8_t fonte_h_min[7] = {0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001};
static const uint8_t fonte_i_min[7] = {0b00100, 0b00000, 0b01100, 0b00100, 0b00100, 0b00100, 0b01110};
static const uint8_t fonte_j_min[7] = {0b00010, 0b00000, 0b00110, 0b00010, 0b00010, 0b10010, 0b01100};
static const uint8_t fonte_k_min[7] = {0b10000, 0b10000, 0b10010, 0b10100, 0b11000, 0b10100, 0b10001};
static const uint8_t fonte_l_min[7] = {0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
static const uint8_t fonte_m_min[7] = {0b00000, 0b00000, 0b11010, 0b10101, 0b10101, 0b10001, 0b10001};
static const uint8_t fonte_n_min[7] = {0b00000, 0b00000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001};
static const uint8_t fonte_o_min[7] = {0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110};
static const uint8_t fonte_p_min[7] = {0b00000, 0b10110, 0b11001, 0b10001, 0b11001, 0b10000, 0b10000};
static const uint8_t fonte_q_min[7] = {0b00000, 0b01101, 0b10011, 0b10001, 0b01111, 0b00001, 0b00001};
static const uint8_t fonte_r_min[7] = {0b00000, 0b00000, 0b10110, 0b11001, 0b10000, 0b10000, 0b10000};
static const uint8_t fonte_s_min[7] = {0b00000, 0b00000, 0b01111, 0b10000, 0b01110, 0b00001, 0b11110};
static const uint8_t fonte_t_min[7] = {0b00100, 0b00100, 0b01110, 0b00100, 0b00100, 0b00100, 0b00011};
static const uint8_t fonte_u_min[7] = {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b10011, 0b01101};
static const uint8_t fonte_v_min[7] = {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100};
static const uint8_t fonte_w_min[7] = {0b00000, 0b00000, 0b10001, 0b10001, 0b10101, 0b10101, 0b01010};
static const uint8_t fonte_x_min[7] = {0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001};
static const uint8_t fonte_y_min[7] = {0b00000, 0b10001, 0b10001, 0b01111, 0b00001, 0b10001, 0b01110};
static const uint8_t fonte_z_min[7] = {0b00000, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111};

// Fontes 5x7 dos Números 0-9
static const uint8_t fonte_0[7] = {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110};
static const uint8_t fonte_1[7] = {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
static const uint8_t fonte_2[7] = {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111};
static const uint8_t fonte_3[7] = {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110};
static const uint8_t fonte_4[7] = {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010};
static const uint8_t fonte_5[7] = {0b11111, 0b10000, 0b10000, 0b11110, 0b00001, 0b00001, 0b11110};
static const uint8_t fonte_6[7] = {0b01110, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110};
static const uint8_t fonte_7[7] = {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000};
static const uint8_t fonte_8[7] = {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110};
static const uint8_t fonte_9[7] = {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110};
static void st7735_command(st7735_t *dev, uint8_t command) {
    gpio_set_level(dev->pin_dc, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &command
    };
    spi_device_transmit(dev->spi, &t);
}

static void st7735_data(st7735_t *dev, const uint8_t *data, int length) {
    gpio_set_level(dev->pin_dc, 1);
    spi_transaction_t t = {
        .length = length * 8,
        .tx_buffer = data
    };
    spi_device_transmit(dev->spi, &t);
}

static void st7735_data_byte(st7735_t *dev, uint8_t data) {
    st7735_data(dev, &data, 1);
}

void st7735_reset(st7735_t *dev) {
    gpio_set_level(dev->pin_reset, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(dev->pin_reset, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void st7735_set_window(st7735_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];

    st7735_command(dev, ST7735_CASET);
    data[0] = x0 >> 8; data[1] = x0 & 0xFF;
    data[2] = x1 >> 8; data[3] = x1 & 0xFF;
    st7735_data(dev, data, 4);

    st7735_command(dev, ST7735_RASET);
    data[0] = y0 >> 8; data[1] = y0 & 0xFF;
    data[2] = y1 >> 8; data[3] = y1 & 0xFF;
    st7735_data(dev, data, 4);

    st7735_command(dev, ST7735_RAMWR);
}

void st7735_draw_pixel(st7735_t *dev, uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT) return;

    st7735_set_window(dev, x, y, x, y);
    uint8_t data[2] = { color >> 8, color & 0xFF };
    st7735_data(dev, data, 2);
}

void st7735_fill_rect(st7735_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT)) return;
    if ((x + w - 1) >= ST7735_WIDTH) w = ST7735_WIDTH - x;
    if ((y + h - 1) >= ST7735_HEIGHT) h = ST7735_HEIGHT - y;

    st7735_set_window(dev, x, y, x + w - 1, y + h - 1);

    uint8_t buffer[w * 2];
    for (int i = 0; i < w; i++) {
        buffer[i * 2] = color >> 8;
        buffer[i * 2 + 1] = color & 0xFF;
    }

    for (int row = 0; row < h; row++) {
        st7735_data(dev, buffer, sizeof(buffer));
    }
}

void st7735_fill_screen(st7735_t *dev, uint16_t color) {
    st7735_fill_rect(dev, 0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

void st7735_draw_char(st7735_t *dev, uint16_t x, uint16_t y, char caractere, uint16_t color) {
    const uint8_t *fonte = NULL;

    switch (caractere) {
        // Letras Maiúsculas
        case 'A': fonte = fonte_A; break;
        case 'B': fonte = fonte_B; break;
        case 'C': fonte = fonte_C; break;
        case 'D': fonte = fonte_D; break;
        case 'E': fonte = fonte_E; break;
        case 'F': fonte = fonte_F; break;
        case 'G': fonte = fonte_G; break;
        case 'H': fonte = fonte_H; break;
        case 'I': fonte = fonte_I; break;
        case 'J': fonte = fonte_J; break;
        case 'K': fonte = fonte_K; break;
        case 'L': fonte = fonte_L; break;
        case 'M': fonte = fonte_M; break;
        case 'N': fonte = fonte_N; break;
        case 'O': fonte = fonte_O; break;
        case 'P': fonte = fonte_P; break;
        case 'Q': fonte = fonte_Q; break;
        case 'R': fonte = fonte_R; break;
        case 'S': fonte = fonte_S; break;
        case 'T': fonte = fonte_T; break;
        case 'U': fonte = fonte_U; break;
        case 'V': fonte = fonte_V; break;
        case 'W': fonte = fonte_W; break;
        case 'X': fonte = fonte_X; break;
        case 'Y': fonte = fonte_Y; break;
        case 'Z': fonte = fonte_Z; break;

        // Letras Minúsculas
        case 'a': fonte = fonte_a_min; break;
        case 'b': fonte = fonte_b_min; break;
        case 'c': fonte = fonte_c_min; break;
        case 'd': fonte = fonte_d_min; break;
        case 'e': fonte = fonte_e_min; break;
        case 'f': fonte = fonte_f_min; break;
        case 'g': fonte = fonte_g_min; break;
        case 'h': fonte = fonte_h_min; break;
        case 'i': fonte = fonte_i_min; break;
        case 'j': fonte = fonte_j_min; break;
        case 'k': fonte = fonte_k_min; break;
        case 'l': fonte = fonte_l_min; break;
        case 'm': fonte = fonte_m_min; break;
        case 'n': fonte = fonte_n_min; break;
        case 'o': fonte = fonte_o_min; break;
        case 'p': fonte = fonte_p_min; break;
        case 'q': fonte = fonte_q_min; break;
        case 'r': fonte = fonte_r_min; break;
        case 's': fonte = fonte_s_min; break;
        case 't': fonte = fonte_t_min; break;
        case 'u': fonte = fonte_u_min; break;
        case 'v': fonte = fonte_v_min; break;
        case 'w': fonte = fonte_w_min; break;
        case 'x': fonte = fonte_x_min; break;
        case 'y': fonte = fonte_y_min; break;
        case 'z': fonte = fonte_z_min; break;
        
        // Números
        case '0': fonte = fonte_0; break;
        case '1': fonte = fonte_1; break;
        case '2': fonte = fonte_2; break;
        case '3': fonte = fonte_3; break;
        case '4': fonte = fonte_4; break;
        case '5': fonte = fonte_5; break;
        case '6': fonte = fonte_6; break;
        case '7': fonte = fonte_7; break;
        case '8': fonte = fonte_8; break;
        case '9': fonte = fonte_9; break;

        default: return;
    }

    for (int linha = 0; linha < 7; linha++) {
        uint8_t bits = fonte[linha];
        for (int coluna = 0; coluna < 5; coluna++) {
            if (bits & (1 << (4 - coluna))) {
                st7735_draw_pixel(dev, x + coluna, y + linha, color);
            }
        }
    }
}

void st7735_draw_string(st7735_t *dev, uint16_t x, uint16_t y, const char *texto, uint16_t color) {
    while (*texto) {
        st7735_draw_char(dev, x, y, *texto, color);
        x += 6;
        texto++;
    }
}

void st7735_init(st7735_t *dev, spi_host_device_t host, gpio_num_t sclk, gpio_num_t mosi, gpio_num_t cs, gpio_num_t dc, gpio_num_t rst) {
    dev->pin_dc = dc;
    dev->pin_reset = rst;

    gpio_set_direction(dc, GPIO_MODE_OUTPUT);
    gpio_set_direction(rst, GPIO_MODE_OUTPUT);

    spi_bus_config_t buscfg = {
        .sclk_io_num = sclk,
        .mosi_io_num = mosi,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = ST7735_WIDTH * 2
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = cs,
        .queue_size = 1
    };

    ESP_ERROR_CHECK(spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(host, &devcfg, &dev->spi));

    st7735_reset(dev);
    st7735_command(dev, ST7735_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));

    st7735_command(dev, ST7735_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(150));

    st7735_command(dev, ST7735_COLMOD);
    st7735_data_byte(dev, 0x05);
    vTaskDelay(pdMS_TO_TICKS(10));

    st7735_command(dev, ST7735_DISPON);
    vTaskDelay(pdMS_TO_TICKS(100));
}