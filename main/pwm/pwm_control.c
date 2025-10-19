#include "pwm_control.h"

static const char *TAG = "pwm_mcpwm_new";

static uint32_t duty_percent_to_ticks(float duty_percent, uint32_t period_ticks) {
    if (duty_percent < 0.0f) duty_percent = 0.0f;
    if (duty_percent > 100.0f) duty_percent = 100.0f;
    return (uint32_t)((duty_percent / 100.0f) * period_ticks);
}

void pwm_init(uint32_t freq_hz, int group_id, pwm_instance_t *inst, gpio_num_t pwm_gpio) {
    if (!inst) {
        ESP_LOGE(TAG, "Invalid unit/timer/op");
        return;
    }
    if (inst->initialized) {
        if (inst->gen_h) mcpwm_del_generator(inst->gen_h);
        if (inst->cmpr_h) mcpwm_del_comparator(inst->cmpr_h);
        if (inst->oper_h) mcpwm_del_operator(inst->oper_h);
        if (inst->timer_h) mcpwm_del_timer(inst->timer_h);
        inst->initialized = false;
    }

    if (freq_hz == 0) {
        ESP_LOGE(TAG, "Invalid frequency");
        return;
    }
    inst->period_ticks = MCPWM_RESOLUTION_HZ / freq_hz;
    if (inst->period_ticks == 0) {
        ESP_LOGE(TAG, "Frequency too high for resolution");
        return;
    }

    inst->group_id = group_id;
    mcpwm_timer_config_t timer_cfg = {
        .group_id = group_id,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = MCPWM_RESOLUTION_HZ,
        .period_ticks = inst->period_ticks,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_cfg, &inst->timer_h));
    // 该定时器将向上计数，从 0 计数到 period_ticks，然后重新开始，重装填频率为 freq_hz

    mcpwm_operator_config_t oper_cfg = {
        .group_id = group_id,
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&oper_cfg, &inst->oper_h));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(inst->oper_h, inst->timer_h));

    mcpwm_comparator_config_t cmpr_cfg = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(inst->oper_h, &cmpr_cfg, &inst->cmpr_h));

    mcpwm_generator_config_t gen_cfg = {
        .gen_gpio_num = pwm_gpio,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(inst->oper_h, &gen_cfg, &inst->gen_h));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(
        inst->gen_h,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()
    ));
    // 每当计时器计数到达零点时，输出设置为低电平；当计数到达周期顶点时，输出设置为高电平
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        inst->gen_h,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, inst->cmpr_h, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()
    ));
    // 在比较器值处将输出设置为高电平，形成低电平脉冲

    ESP_ERROR_CHECK(mcpwm_timer_enable(inst->timer_h));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(inst->timer_h, MCPWM_TIMER_START_NO_STOP));

    inst->last_duty_percent = 0.0f;
    inst->initialized = true;
    ESP_LOGI(TAG, "PWM initialized: group=%d freq=%u gpio=%d", group_id, freq_hz, pwm_gpio);
}

void pwm_init_conj(uint32_t freq_hz, int group_id, pwm_instance_t *inst, gpio_num_t pwm_gpio, pwm_instance_t *inst_conj, gpio_num_t pwm_gpio_conj) {
    if (!inst || !inst_conj) {
        ESP_LOGE(TAG, "Invalid pwm_instance pointer");
        return;
    }
    // 清理旧实例
    if (inst->initialized) {
        if (inst->gen_h) mcpwm_del_generator(inst->gen_h);
        if (inst->cmpr_h) mcpwm_del_comparator(inst->cmpr_h);
        if (inst->oper_h) mcpwm_del_operator(inst->oper_h);
        if (inst->timer_h) mcpwm_del_timer(inst->timer_h);
        inst->initialized = false;
    }
    if (inst_conj->initialized) {
        if (inst_conj->gen_h) mcpwm_del_generator(inst_conj->gen_h);
        inst_conj->initialized = false;
    }
    if (freq_hz == 0) {
        ESP_LOGE(TAG, "Invalid frequency");
        return;
    }
    uint32_t period_ticks = MCPWM_RESOLUTION_HZ / freq_hz;
    if (period_ticks == 0) {
        ESP_LOGE(TAG, "Frequency too high for resolution");
        return;
    }
    // 只创建一次 timer/oper/cmpr，两个实例共用
    inst->group_id = group_id;
    mcpwm_timer_config_t timer_cfg = {
        .group_id = group_id,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = MCPWM_RESOLUTION_HZ,
        .period_ticks = period_ticks,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_cfg, &inst->timer_h));
    mcpwm_operator_config_t oper_cfg = {
        .group_id = group_id,
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&oper_cfg, &inst->oper_h));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(inst->oper_h, inst->timer_h));
    mcpwm_comparator_config_t cmpr_cfg = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(inst->oper_h, &cmpr_cfg, &inst->cmpr_h));
    // 主输出生成器
    mcpwm_generator_config_t gen_cfg = {
        .gen_gpio_num = pwm_gpio,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(inst->oper_h, &gen_cfg, &inst->gen_h));
    // 共轭输出生成器
    mcpwm_generator_config_t gen_cfg_conj = {
        .gen_gpio_num = pwm_gpio_conj,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(inst->oper_h, &gen_cfg_conj, &inst_conj->gen_h));
    // 设置主输出：空时低，满时高，比较点高
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(
        inst->gen_h,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()
    ));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        inst->gen_h,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, inst->cmpr_h, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()
    ));
    // 设置共轭输出：空时高，满时低，比较点低（与主输出相反）
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(
        inst_conj->gen_h,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()
    ));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        inst_conj->gen_h,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, inst->cmpr_h, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()
    ));
    // 使能并启动
    ESP_ERROR_CHECK(mcpwm_timer_enable(inst->timer_h));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(inst->timer_h, MCPWM_TIMER_START_NO_STOP));
    // 两个实例共享 timer/oper/cmpr，独立 gen_h
    inst->period_ticks = period_ticks;
    inst->last_duty_percent = 0.0f;
    inst->initialized = true;
    inst_conj->timer_h = inst->timer_h;
    inst_conj->oper_h = inst->oper_h;
    inst_conj->cmpr_h = inst->cmpr_h;
    inst_conj->period_ticks = period_ticks;
    inst_conj->last_duty_percent = 0.0f;
    inst_conj->initialized = true;
    inst_conj->group_id = group_id;
    ESP_LOGI(TAG, "PWM conj initialized: group=%d freq=%u gpio(main)=%d gpio(conj)=%d", group_id, freq_hz, pwm_gpio, pwm_gpio_conj);
}

void pwm_set(float duty_percent, pwm_instance_t *inst) {
    if (!inst || !inst->initialized) {
        ESP_LOGE(TAG, "PWM not initialized");
        return;
    }

    if (duty_percent < 0.0) duty_percent = 0.0;
    if (duty_percent > 100.0) duty_percent = 100.0;

    uint32_t cmp_ticks = duty_percent_to_ticks((float)duty_percent, inst->period_ticks);
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(inst->cmpr_h, cmp_ticks));

    inst->last_duty_percent = (float)duty_percent;
}

void pwm_stop(pwm_instance_t *inst) {
    if (!inst || !inst->initialized) return;
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(inst->timer_h, MCPWM_TIMER_STOP_EMPTY));
    ESP_ERROR_CHECK(mcpwm_timer_disable(inst->timer_h));
}

float get_pwm_duty(pwm_instance_t *inst) {
    if (!inst || !inst->initialized) return 0.0f;
    return inst->last_duty_percent;
}