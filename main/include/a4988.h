#ifndef A4988_H
#define A4988_H

#include <stdint.h>
#include "driver/gpio.h"

// Definições de Sentido de Rotação
#define A4988_DIR_HOR   0  // Sentido Horário
#define A4988_DIR_ANT  1  // Sentido Anti-Horário

// Estrutura para modularizar o motor
typedef struct {
    gpio_num_t pin_step;
    gpio_num_t pin_dir;
    gpio_num_t pin_enable;
    uint32_t delay_us;     // Intervalo entre pulsos em microssegundos (controla a velocidade)
} a4988_t;

// Assinaturas das Funções
void a4988_init(a4988_t *dev, gpio_num_t step, gpio_num_t dir, gpio_num_t enable, uint32_t delay_us);
void a4988_enable(a4988_t *dev);
void a4988_disable(a4988_t *dev);
void a4988_step(a4988_t *dev, uint32_t passos, uint8_t direcao);
void a4988_set_delay(a4988_t *dev, uint32_t delay_us);

#endif