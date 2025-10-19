#pragma once

#include "driver/i2c.h"
#include <string.h>
#include "global_params.h"
#include "esp_timer.h"

#define I2C_BUF_SIZE                1024
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_CONTENT_BUFFER_SIZE     10
#define I2C_PORTS_MAX               2

typedef struct {
	uint8_t i2c_num;
	uint8_t addr;
	uint8_t data[I2C_BUF_SIZE];
	int length;
} i2c_content_t;

typedef struct {
	i2c_port_t i2c_num;
	uint8_t addr;
} i2c_device_t;

void i2c_timer_service_start(void);
void i2c_add_device(i2c_port_t i2c_num, uint8_t addr);
i2c_content_t* i2c_read_buffer(void);
void i2c_init(i2c_port_t i2c_num, gpio_num_t sda_io, gpio_num_t scl_io);
esp_err_t i2c_read(i2c_port_t i2c_num, uint8_t addr, uint8_t* data, size_t len);
esp_err_t i2c_write(i2c_port_t i2c_num, uint8_t dev_addr, const uint8_t *data, size_t len);

esp_err_t i2c_read_reg(i2c_port_t i2c_num, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t i2c_write_reg(i2c_port_t i2c_num, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len);