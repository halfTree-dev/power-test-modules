#include <stdio.h>
#include "global_params.h"
#include "gpio/gpio_control.h"
#include "uart/uart_control.h"
#include "pwm/pwm_control.h"
#include "i2c/i2c_control.h"
#include "i2c_oled/i2c_oled_control.h"

static const char *TAG = "main";

void app_main(void) {
    gpio_init(GPIO_NUM_2, GPIO_MODE_OUTPUT, 0);

    uart_timer_service_init();
    uart_init(UART_NUM_0);

    pwm_instance_t pwm_inst = {0};
    pwm_instance_t pwm_inst_conj = {0};
    pwm_init_conj(20000, 0, &pwm_inst, GPIO_NUM_51, &pwm_inst_conj, GPIO_NUM_52);

    i2c_timer_service_start();
    i2c_init(I2C_NUM_0, GPIO_NUM_19, GPIO_NUM_18);
    i2c_init(I2C_NUM_1, GPIO_NUM_21, GPIO_NUM_22);

    OLED_init();
    OLED_update();

    ESP_LOGI(TAG, "POWER TEST MODULES START\n");

    while (1) {
        uart_content_t* cmd = uart_read();
        if (cmd != NULL) {
            ESP_LOGI(TAG, "UART%d", cmd->uart_num);
            ESP_LOGI(TAG, "Received command: %s", cmd->data);
            float duty = strtof((const char*)cmd->data, NULL);
            if (duty >= 0.0f && duty <= 100.0f) {
                pwm_set(duty, &pwm_inst);
                ESP_LOGI(TAG, "Set PWM duty: %.2f%%", duty);
                OLED_clear();
                OLED_show_string(0, 0, "PWM Duty:", OLED_6X8);
                char duty_str[16];
                snprintf(duty_str, sizeof(duty_str), "%.2f%%", duty);
                OLED_show_string(0, 16, duty_str, OLED_6X8);
                OLED_update();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}