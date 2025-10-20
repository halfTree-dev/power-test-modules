#pragma once

#include "global_params.h"
#include "driver/mcpwm_prelude.h"
#include "driver/mcpwm_types.h"
#include "esp_log.h"

#define MCPWM_RESOLUTION_HZ (30000000) // 30 MHz 分辨率

typedef struct {
    int group_id;
    mcpwm_timer_handle_t timer_h;
    mcpwm_oper_handle_t oper_h;
    mcpwm_cmpr_handle_t cmpr_h;
    mcpwm_gen_handle_t gen_h;
    float last_duty_percent;
    uint32_t period_ticks;
    bool initialized;
} pwm_instance_t;

void pwm_init(uint32_t freq_hz, int group_id, pwm_instance_t *inst, gpio_num_t pwm_gpio);
void pwm_init_conj(uint32_t freq_hz, int group_id, pwm_instance_t *inst, gpio_num_t pwm_gpio, pwm_instance_t *inst_conj, gpio_num_t pwm_gpio_conj);
void pwm_set(float duty_percent, pwm_instance_t *inst);
void pwm_stop(pwm_instance_t *inst);
float get_pwm_duty(pwm_instance_t *inst);