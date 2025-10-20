# Power Test Modules - ESP32-P4 电源控制系统

[![Platform](https://img.shields.io/badge/platform-ESP32--P4-blue.svg)](https://github.com/espressif/esp-idf)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.0.0-orange.svg)](README.md)

本项目是为全国大学生电子设计竞赛的电源题目设计的软件模板，基于 ESP32-P4 微控制器，旨在简化电源管理和控制任务的实现。

## 已实现的功能

### 标准外设驱动

- [x] GPIO 控制

### 通信模块

- [x] UART 通信
- [x] I2C 主机通信
    - [x] INA226/228 驱动模块
    - [x] SSD1306 OLED 驱动模块
- [x] SPI 主机通信

### 脉宽调制

- [x] PWM 波生成
- [x] PWM 波互补生成

### 控制算法

- [x] PID 控制

## 正在计划实现的功能

- 有效值检波
- SPWM 调制
- 数字滤波

> Made by half-tree