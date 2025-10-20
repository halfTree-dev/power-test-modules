#include <stdio.h>
#include <string.h>
#include "global_params.h"
#include "gpio/gpio_control.h"
#include "uart/uart_control.h"
#include "pwm/pwm_control.h"
#include "i2c/i2c_control.h"
#include "i2c_oled/i2c_oled_control.h"
#include "i2c_ina226_driver/i2c_ina226_driver.h"
#include "pid/pid_control.h"

static const char *TAG = "main";

void Show_OLED_Content(float target_v_out, float v_bus, float i_measure, float pwm_duty);

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

    ina226_data_t ina226_data = {0};
    ina226_init();

    float target_bus_voltage = 10.0f;
    float current_pwm_duty = 0.0f;
    float current_bus_voltage = 0.0f;
    pid_handle_t pid = {0};
    pid_init(&pid, target_bus_voltage, &current_pwm_duty, &current_bus_voltage);
    pid_timer_service_init(&pid);

    while (1) {
        uart_content_t* cmd = uart_read();
        if (cmd != NULL) {
            ESP_LOGI(TAG, "Received command: %s", cmd->data);
            char *cmd_str = (char*)cmd->data;

            // 处理Reset命令
            if (strcmp(cmd_str, "R") == 0) {
                pid_reset(&pid);
                ESP_LOGI(TAG, "PID reset completed");
            }
            // 处理电压设置命令 V:<float>
            else if (strncmp(cmd_str, "V:", 2) == 0) {
                float set_voltage = strtof(cmd_str + 2, NULL);
                if (set_voltage >= 10.0f && set_voltage <= 18.0f) {
                    target_bus_voltage = set_voltage;
                    ESP_LOGI(TAG, "Set target bus voltage: %.2fV", set_voltage);
                    change_pid_setpoint(&pid, target_bus_voltage);
                }
            }
            // 处理PID参数设置命令 K:<P/I/D>:<float>
            else if (strncmp(cmd_str, "K:", 2) == 0) {
                char param_type = cmd_str[2];
                if (cmd_str[3] == ':') {
                    float param_value = strtof(cmd_str + 4, NULL);
                    switch (param_type) {
                        case 'P':
                            pid_set_kp(&pid, param_value);
                            ESP_LOGI(TAG, "Set Kp: %.3f", param_value);
                            break;
                        case 'I':
                            pid_set_ki(&pid, param_value);
                            ESP_LOGI(TAG, "Set Ki: %.3f", param_value);
                            break;
                        case 'D':
                            pid_set_kd(&pid, param_value);
                            ESP_LOGI(TAG, "Set Kd: %.3f", param_value);
                            break;
                        default:
                            ESP_LOGW(TAG, "Invalid PID parameter: %c", param_type);
                            break;
                    }
                }
            }
            // 兼容原有的直接数字输入（作为电压设置）
            else {
                float set_voltage = strtof(cmd_str, NULL);
                if (set_voltage >= 10.0f && set_voltage <= 18.0f) {
                    target_bus_voltage = set_voltage;
                    ESP_LOGI(TAG, "Set target bus voltage: %.2fV", set_voltage);
                    change_pid_setpoint(&pid, target_bus_voltage);
                }
            }
        }

        pwm_set(current_pwm_duty, &pwm_inst);

        ina226_read_all(&ina226_data);
        current_bus_voltage = ina226_data.bus_voltage_v;

        Show_OLED_Content(target_bus_voltage, current_bus_voltage, ina226_data.current_ma / 1000.0f, current_pwm_duty);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void Show_OLED_Content(float target_v_out, float v_bus, float i_measure, float pwm_duty) {
    OLED_clear();
    char buf[24];
    snprintf(buf, sizeof(buf), "V_target: %.3fV", target_v_out);
    OLED_show_string(0, 0, buf, OLED_6X8);
    snprintf(buf, sizeof(buf), "V_measure: %.4fV", v_bus);
    OLED_show_string(0, 16, buf, OLED_6X8);
    snprintf(buf, sizeof(buf), "I_measure: %.5fA", i_measure);
    OLED_show_string(0, 32, buf, OLED_6X8);
    snprintf(buf, sizeof(buf), "PWM Duty: %.3f%%", pwm_duty);
    OLED_show_string(0, 48, buf, OLED_6X8);
    OLED_update();
}