#include "i2c_control.h"

static esp_timer_handle_t i2c_timer;
static i2c_content_t i2c_content_buffer[I2C_CONTENT_BUFFER_SIZE];
static volatile int i2c_write_index = 0;
static volatile int i2c_read_index = 0;
static volatile int i2c_content_count = 0;

static i2c_device_t opened_i2c_addrs[I2C_PORTS_MAX];
static volatile int opened_i2c_count = 0;

void i2c_timer_callback(void* arg)
{
	for (int i = 0; i < opened_i2c_count; ++i) {
		uint8_t addr = opened_i2c_addrs[i].addr;
		i2c_port_t i2c_num = opened_i2c_addrs[i].i2c_num;
		uint8_t data[I2C_BUF_SIZE] = {0};
		int len = I2C_BUF_SIZE;
		esp_err_t ret = i2c_read(i2c_num, addr, data, len);
		if (ret == ESP_OK && i2c_content_count < I2C_CONTENT_BUFFER_SIZE) {
			memcpy(i2c_content_buffer[i2c_write_index].data, data, len);
			i2c_content_buffer[i2c_write_index].addr = addr;
			i2c_content_buffer[i2c_write_index].length = len;
			i2c_write_index = (i2c_write_index + 1) % I2C_CONTENT_BUFFER_SIZE;
			i2c_content_count++;
		}
	}
}

void i2c_timer_service_start(void)
{
	const esp_timer_create_args_t timer_args = {
		.callback = &i2c_timer_callback,
		.name = "i2c_timer"
	};
	ESP_ERROR_CHECK(esp_timer_create(&timer_args, &i2c_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(i2c_timer, 50000));
}

void i2c_add_device(i2c_port_t i2c_num, uint8_t addr)
{
	if (opened_i2c_count < I2C_PORTS_MAX) {
		opened_i2c_addrs[opened_i2c_count++] = (i2c_device_t){i2c_num, addr};
	}
}

i2c_content_t* i2c_read_buffer(void)
{
	if (i2c_content_count <= 0) {
		return NULL;
	}
	i2c_content_t* content = &i2c_content_buffer[i2c_read_index];
	i2c_read_index = (i2c_read_index + 1) % I2C_CONTENT_BUFFER_SIZE;
	i2c_content_count--;
	return content;
}

void i2c_init(i2c_port_t i2c_num, gpio_num_t sda_io, gpio_num_t scl_io)
{
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = sda_io,
		.scl_io_num = scl_io,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_FREQ_HZ,
	};
	ESP_ERROR_CHECK(i2c_param_config(i2c_num, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(i2c_num, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));
}

esp_err_t i2c_write(i2c_port_t i2c_num, uint8_t dev_addr, const uint8_t *data, size_t len)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write(cmd, (uint8_t*)data, len, true);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	return ret;
}

esp_err_t i2c_read(i2c_port_t i2c_num, uint8_t dev_addr, uint8_t *data, size_t len)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
	i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	return ret;
}

esp_err_t i2c_read_reg(i2c_port_t i2c_num, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t i2c_write_reg(i2c_port_t i2c_num, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, (uint8_t*)data, len, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}