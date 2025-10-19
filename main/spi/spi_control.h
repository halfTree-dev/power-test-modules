#pragma once

#include "driver/spi_master.h"
#include <stdint.h>
#include <stddef.h>

#define SPI_BUF_SIZE 256
#define SPI_PORTS_MAX 2

typedef struct {
    spi_device_handle_t handle;
    uint8_t data[SPI_BUF_SIZE];
    int length;
} spi_content_t;

void spi_init(int host, int sclk_io, int mosi_io, int miso_io, int cs_io, spi_device_handle_t *handle);
int spi_write(spi_device_handle_t handle, const uint8_t *data, size_t len);
int spi_read(spi_device_handle_t handle, uint8_t *data, size_t len);
void spi_add_device(spi_device_handle_t handle);
void spi_timer_service_start(void);
spi_content_t* spi_read_buffer(void);
