#ifndef ST7735_H
#define ST7735_H

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"

// Resolução da Tela
#define ST7735_WIDTH   128
#define ST7735_HEIGHT  160

// Comandos do ST7735
#define ST7735_SWRESET  0x01
#define ST7735_SLPOUT   0x11
#define ST7735_COLMOD   0x3A
#define ST7735_DISPON   0x29
#define ST7735_CASET    0x2A
#define ST7735_RASET    0x2B
#define ST7735_RAMWR    0x2C

// Cores Básicas RGB565
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW 0xFFE0
#define COLOR_GRAY   0x8410

// Estrutura do Display
typedef struct {
    spi_device_handle_t spi;
    gpio_num_t pin_dc;
    gpio_num_t pin_reset;
} st7735_t;

// Assinaturas das Funções (Padrão st7735_)
void st7735_init(st7735_t *dev, spi_host_device_t host, gpio_num_t sclk, gpio_num_t mosi, gpio_num_t cs, gpio_num_t dc, gpio_num_t rst);
void st7735_reset(st7735_t *dev);
void st7735_draw_pixel(st7735_t *dev, uint16_t x, uint16_t y, uint16_t color);
void st7735_fill_screen(st7735_t *dev, uint16_t color);
void st7735_fill_rect(st7735_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7735_draw_char(st7735_t *dev, uint16_t x, uint16_t y, char caractere, uint16_t color);
void st7735_draw_string(st7735_t *dev, uint16_t x, uint16_t y, const char *texto, uint16_t color);

#endif