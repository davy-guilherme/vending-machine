#include "a4988.h"
#include "esp_rom_sys.h" // esp_rom_delay_us

void a4988_init(a4988_t *dev, gpio_num_t step, gpio_num_t dir, gpio_num_t enable, uint32_t delay_us) {
    dev->pin_step = step;
    dev->pin_dir = dir;
    dev->pin_enable = enable;
    dev->delay_us = delay_us;

    // Configuração dos GPIOs
    gpio_config_t io_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << step) | (1ULL << dir) | (1ULL << enable),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_config);

    // Estado Inicial (Desabilitado por padrão)
    gpio_set_level(dev->pin_step, 0);
    gpio_set_level(dev->pin_dir, 0);
    a4988_disable(dev); // EN em HIGH desabilita o driver
}

void a4988_enable(a4988_t *dev) {
    // Pino ENABLE no A4988 é ativo em LOW (0)
    gpio_set_level(dev->pin_enable, 0);
}

void a4988_disable(a4988_t *dev) {
    // Desabilita o driver (1), cortando a corrente e reduzindo o aquecimento
    gpio_set_level(dev->pin_enable, 1);
}

void a4988_step(a4988_t *dev, uint32_t passos, uint8_t direcao) {
    // Garante que o motor está habilitado antes de girar
    a4988_enable(dev);

    // Define o sentido de rotação
    gpio_set_level(dev->pin_dir, direcao);

    // Envia os pulsos de passo
    for (uint32_t i = 0; i < passos; i++) {
        gpio_set_level(dev->pin_step, 1);
        esp_rom_delay_us(dev->delay_us);

        gpio_set_level(dev->pin_step, 0);
        esp_rom_delay_us(dev->delay_us);
    }
}

void a4988_set_delay(a4988_t *dev, uint32_t delay_us) {
    dev->delay_us = delay_us;
}