#pragma once

#include "driver/gpio.h"
#include "global_params.h"

void gpio_init(gpio_num_t target_gpio, gpio_mode_t mode, uint32_t level);
void gpio_set(gpio_num_t target_gpio, uint32_t level);