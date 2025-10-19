idf_component_register(
    SRCS
        "main.c"
        "gpio/gpio_control.c"
        "pwm/pwm_control.c"
        "uart/uart_control.c"
        "i2c/i2c_control.c"
        "i2c_oled/i2c_oled_control.c"
        "spi/spi_control.c"
        "i2c_ina226_driver/i2c_ina226_driver.c"
        "pid/pid_control.c"
    INCLUDE_DIRS "."
)