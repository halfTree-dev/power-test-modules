#include <math.h>
#include "pid_control.h"

static esp_timer_handle_t s_pid_timer = NULL;

// 初始化PID参数，周期由PERIOD_US宏定义
void pid_init(pid_handle_t *pid, float setpoint, float *input_ptr, float *output_ptr)
{
	if (!pid) return;
	pid->kp = PID_KP;
	pid->ki = PID_KI;
	pid->kd = PID_KD;
	pid->input_min = INPUT_LIMIT_MIN;
	pid->input_max = INPUT_LIMIT_MAX;
	pid->integral = 0.0f;
	pid->last_error = 0.0f;
	pid->input_ptr = input_ptr;
	pid->output_ptr = output_ptr;
	pid->setpoint = setpoint;
	pid->period_s = PERIOD_US / 1000000.0f;
}

#ifndef EPSILON
#define EPSILON 1e-6f
#endif

void pid_timer_isr(pid_handle_t *pid)
{
	if (!pid || !pid->input_ptr || !pid->output_ptr) return;

	float output = *(pid->output_ptr);
	if (fabsf(output) < EPSILON) {
		return;
	}
	if (output < pid->input_min) output = pid->input_min;
	if (output > pid->input_max) output = pid->input_max;

    // 积分这一块
	float error = pid->setpoint - output;
	pid->integral += error * pid->period_s;
    if (pid->integral < INTEGRAL_MIN) pid->integral = INTEGRAL_MIN;
    if (pid->integral > INTEGRAL_MAX) pid->integral = INTEGRAL_MAX;

	// 微分这一块
	float derivative = (error - pid->last_error) / pid->period_s;
	pid->last_error = error;
	float input = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;

	// 输入限幅
	if (input < pid->input_min) input = pid->input_min;
	if (input > pid->input_max) input = pid->input_max;
	*(pid->input_ptr) = input;
}

// 定时器服务
static void pid_timer_callback(void *arg)
{
	pid_handle_t *pid = (pid_handle_t *)arg;
	pid_timer_isr(pid);
}

void pid_timer_service_init(pid_handle_t *pid)
{
	if (!pid) return;
	if (s_pid_timer) esp_timer_stop(s_pid_timer);
	esp_timer_create_args_t timer_args = {
		.callback = pid_timer_callback,
		.arg = pid,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "pid_timer"
	};
	esp_timer_create(&timer_args, &s_pid_timer);
	esp_timer_start_periodic(s_pid_timer, PERIOD_US);
}

void change_pid_setpoint(pid_handle_t *pid, float new_setpoint)
{
    if (!pid) return;
    pid->setpoint = new_setpoint;
}

void pid_reset(pid_handle_t *pid)
{
    if (!pid) return;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
}

void pid_set_kp(pid_handle_t *pid, float kp)
{
    if (!pid) return;
    pid->kp = kp;
}

void pid_set_ki(pid_handle_t *pid, float ki)
{
    if (!pid) return;
    pid->ki = ki;
}

void pid_set_kd(pid_handle_t *pid, float kd)
{
    if (!pid) return;
    pid->kd = kd;
}