// ESP32 INA226 电流/电压/功率传感器驱动头文件
// Made By half-tree

#pragma once

#include "i2c/i2c_control.h"
#include "esp_log.h"
#include "esp_err.h"
#include <stdint.h>

#define I2C_INA226_NUM      I2C_NUM_1

// INA226 默认 I2C 地址 (A1=GND, A0=GND)
#define INA226_I2C_ADDR     0x40

// INA226 寄存器地址
#define INA226_REG_CONFIG   0x00  // 配置寄存器
// 配置寄存器决定 INA226 的工作模式、采样时间和平均次数
// 0x4127 是默认值，表示连续测量总线和分流电压，1次平均
#define INA226_REG_SHUNT_V  0x01  // 分流电压寄存器
// 储存采样电阻两端压差
// shunt_V = value * LSB，value 为寄存器读出的原始值
// LSB 是手册中定义的分辨率，为 2.5 * 10^(-6) V/bit
// shunt_V 是计算得到的分流电压，单位为伏
#define INA226_REG_BUS_V    0x02  // 总线电压寄存器
// 总线电压指的是 BUS 引脚到 GND 之间的电压
// bus_V = value * LSB，value 为寄存器读出的原始值
// LSB 是手册中定义的分辨率，为 1.25 * 10^(-3) V/bit
// bus_V 是计算得到的总线电压，单位为伏
#define INA226_REG_POWER    0x03  // 功率寄存器
// 功率寄存器存储通过采样电阻的功率值
// 根据手册，这个值是根据以下公式计算得到的：value = current * bus_V / 20000
// 读取到的寄存器值为 value，则 power = value * Power_LSB，其中 Power_LSB = 25 mV/bit
// power 是计算得到的功率值，单位为毫瓦
#define INA226_REG_CURRENT  0x04  // 电流寄存器
// 电流寄存器存储通过采样电阻的电流值
// 根据手册，这个值是根据以下公式计算得到的： value = shunt_V * Calibration / 2048（可以根据欧姆定律推出）
// 读取到的寄存器值为 value，则 current = value * Current_LSB，其中 Current_LSB = 0.1mA
// current 是计算得到的电流值，单位为毫安
#define INA226_REG_CALIB    0x05  // 校准寄存器
// 校准寄存器用于设置电流测量的比例因子
// CAL = 0.00512 / (Current_LSB * Rshunt)
// 其中 Current_LSB = Max_Current / 32768，电流单位为安培
// Rshunt 是采样电阻值，单位为欧姆

#define INA226_CONFIG_VALUE  0x4127  // 配置寄存器：连续测量、4次平均、1.1ms转换时间
#define SHUNT_RESISTOR_OHMS  0.01f    // 分流电阻 10mΩ
#define MAX_CURRENT_A        3.2768f // 最大电流 3.2768A

#define SHUNT_LSB            0.0025f // 分流电压分辨率 2.5μV/bit，最终输出单位为 mV
#define BUS_LSB              0.00125f // 总线电压分辨率 1.25mV/bit，最终输出单位为 V
#define CURRENT_LSB          0.1f     // 电流分辨率 0.1mA/bit，最终输出单位为 mA
#define POWER_LSB            2.5f     // 功率分辨率 2.5mW/bit，最终输出单位为 mW

typedef struct {
    float bus_voltage_v;     // 总线电压 (V)
    float shunt_voltage_mv;  // 分流电压 (mV)
    float current_ma;        // 电流 (mA)
    float power_mw;          // 功率 (mW)
} ina226_data_t;

esp_err_t ina226_init(void);
esp_err_t ina226_read_all(ina226_data_t *data);
esp_err_t ina226_read_voltage(float *bus_voltage_v);
esp_err_t ina226_read_current(float *current_ma);
esp_err_t ina226_read_power(float *power_mw);