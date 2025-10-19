#include "gpio_control.h"

void gpio_init(gpio_num_t target_gpio, gpio_mode_t mode, uint32_t level)
{
    gpio_reset_pin(target_gpio);
    gpio_set_direction(target_gpio, mode);
    gpio_set_level(target_gpio, level);
}

void gpio_set(gpio_num_t target_gpio, uint32_t level)
{
    gpio_set_level(target_gpio, level);
}