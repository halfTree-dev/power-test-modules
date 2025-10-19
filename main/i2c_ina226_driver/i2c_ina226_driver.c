#include "i2c_ina226_driver.h"

static const char *TAG = "INA226";

esp_err_t ina226_init(void)
{
    // 写 INA226_REG_CONFIG 寄存器
    uint16_t cfg = INA226_CONFIG_VALUE;
    uint8_t config_data[2] = { (uint8_t)((cfg >> 8) & 0xFF), (uint8_t)(cfg & 0xFF) };
    esp_err_t ret = i2c_write_reg(I2C_INA226_NUM, INA226_I2C_ADDR, INA226_REG_CONFIG, config_data, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure INA226: %s", esp_err_to_name(ret));
        return ret;
    }

    // 写 INA226_REG_CALIB 寄存器
    float current_lsb = MAX_CURRENT_A / 32768.0f;
    // Cal = round(0.00512 / (Current_LSB * R_shunt))
    uint32_t calib = (uint32_t)(0.00512f / (current_lsb * SHUNT_RESISTOR_OHMS) + 0.5f);
    if (calib == 0 || calib > 0xFFFF) {
        ESP_LOGW(TAG, "Calculated calib out of range: %u", calib);
    }
    uint16_t calib_u16 = (uint16_t)(calib & 0xFFFF);
    uint8_t calib_data[2] = { (uint8_t)((calib_u16 >> 8) & 0xFF), (uint8_t)(calib_u16 & 0xFF) };
    ret = i2c_write_reg(I2C_INA226_NUM, INA226_I2C_ADDR, INA226_REG_CALIB, calib_data, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set calibration: %s", esp_err_to_name(ret));
        return ret;
    }

    // 初始化成功
    ESP_LOGI(TAG, "INA226 initialized: CONFIG=0x%04X CALIB=0x%04X (Current_LSB=%.6f A/bit, Power_LSB=%.6f W/bit)",
             cfg, calib_u16, current_lsb, 25.0f * current_lsb);
    return ESP_OK;
}

static uint16_t _bytes_to_uint16(uint8_t *data)
{
    return (data[0] << 8) | data[1];
}

static int16_t _bytes_to_int16(uint8_t *data)
{
    return (int16_t)((data[0] << 8) | data[1]);
}

esp_err_t ina226_read_voltage(float *bus_voltage_v)
{
    uint8_t data[2];
    esp_err_t ret = i2c_read_reg(I2C_INA226_NUM, INA226_I2C_ADDR, INA226_REG_BUS_V, data, 2);
    if (ret == ESP_OK) {
        uint16_t raw = _bytes_to_uint16(data);
        *bus_voltage_v = raw * BUS_LSB;
    }
    return ret;
}

esp_err_t ina226_read_current(float *current_ma)
{
    uint8_t data[2];
    esp_err_t ret = i2c_read_reg(I2C_INA226_NUM, INA226_I2C_ADDR, INA226_REG_CURRENT, data, 2);
    if (ret == ESP_OK) {
        int16_t raw = _bytes_to_int16(data);
        *current_ma = raw * CURRENT_LSB;
    }
    return ret;
}

esp_err_t ina226_read_power(float *power_mw)
{
    uint8_t data[2];
    esp_err_t ret = i2c_read_reg(I2C_INA226_NUM, INA226_I2C_ADDR, INA226_REG_POWER, data, 2);
    if (ret == ESP_OK) {
        uint16_t raw = _bytes_to_uint16(data);
        *power_mw = raw * POWER_LSB;
    }
    return ret;
}

esp_err_t ina226_read_all(ina226_data_t *data)
{
    esp_err_t ret;

    // 读取分流电压
    uint8_t shunt_data[2];
    ret = i2c_read_reg(I2C_INA226_NUM, INA226_I2C_ADDR, INA226_REG_SHUNT_V, shunt_data, 2);
    if (ret == ESP_OK) {
        int16_t raw = _bytes_to_int16(shunt_data);
        data->shunt_voltage_mv = raw * SHUNT_LSB;
    } else return ret;

    ret = ina226_read_voltage(&data->bus_voltage_v);
    if (ret != ESP_OK) return ret;

    ret = ina226_read_current(&data->current_ma);
    if (ret != ESP_OK) return ret;

    ret = ina226_read_power(&data->power_mw);
    return ret;
}