#pragma once

#include "../i2c/i2c_control.h"

#define OLED_8X16 8
#define OLED_6X8 6

#define OLED_TYPE        "SSD1306"
#define OLED_I2C_PORT    I2C_NUM_0
#define OLED_I2C_ADDR    0x3C

void OLED_write_command(uint8_t command);
void OLED_write_data(uint8_t *data, uint8_t count);
void OLED_init(void);
void OLED_set_cursor(uint8_t page, uint8_t column);
void OLED_update(void);
void OLED_clear(void);
void OLED_reverse(void);
void OLED_show_image(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void OLED_show_char(int16_t X, int16_t Y, char Char, uint8_t FontSize);
void OLED_show_string(int16_t X, int16_t Y, char *String, uint8_t FontSize);