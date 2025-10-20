#pragma once

#include "esp_timer.h"
#include <stddef.h>

// 默认PID参数宏
#define PID_KP  0.4f
#define PID_KI  0.7f
#define PID_KD  0.0f

// 输入/输出上下界
#define INPUT_LIMIT_MIN   0.0f
#define INPUT_LIMIT_MAX   65.0f

// 如果设备过久不响应，该上下界将用于防止积分项过大
#define INTEGRAL_MIN   -100.0f
#define INTEGRAL_MAX    100.0f

#define PERIOD_US         1000

typedef struct {
	float kp;
	float ki;
	float kd;
	float setpoint;
	float input_min;
	float input_max;
	float integral;
	float last_error;
	float *input_ptr;
	float *output_ptr;
	float period_s;
} pid_handle_t;

// 初始化 PID，setpoint 为目标值，input_ptr / output_ptr 为输入输出指针
void pid_init(pid_handle_t *pid, float setpoint, float *input_ptr, float *output_ptr);

// 启动定时器服务，自动以 PERIOD_US 频率调用 pid_timer_isr
void pid_timer_service_init(pid_handle_t *pid);

// 定时器中断服务函数
void pid_timer_isr(pid_handle_t *pid);

void change_pid_setpoint(pid_handle_t *pid, float new_setpoint);

// 重置PID状态（清空积分和上次误差）
void pid_reset(pid_handle_t *pid);

// 修改PID参数
void pid_set_kp(pid_handle_t *pid, float kp);
void pid_set_ki(pid_handle_t *pid, float ki);
void pid_set_kd(pid_handle_t *pid, float kd);