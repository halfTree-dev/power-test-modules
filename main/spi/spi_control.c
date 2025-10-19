#include "spi_control.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>

#define SPI_CONTENT_BUFFER_SIZE 10

static const char *TAG = "spi_control";
static spi_content_t spi_content_buffer[SPI_CONTENT_BUFFER_SIZE];
static volatile int spi_write_index = 0;
static volatile int spi_read_index = 0;
static volatile int spi_content_count = 0;

static spi_device_handle_t opened_spi_handles[SPI_PORTS_MAX];
static volatile int opened_spi_count = 0;
static esp_timer_handle_t spi_timer;

void spi_init(int host, int sclk_io, int mosi_io, int miso_io, int cs_io, spi_device_handle_t *handle)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = mosi_io,
        .miso_io_num = miso_io,
        .sclk_io_num = sclk_io,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_BUF_SIZE,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO));
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = cs_io,
        .queue_size = 3,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(host, &devcfg, handle));
}

int spi_write(spi_device_handle_t handle, const uint8_t *data, size_t len)
{
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
        .rx_buffer = NULL,
    };
    esp_err_t ret = spi_device_transmit(handle, &t);
    return (ret == ESP_OK) ? len : -1;
}

int spi_read(spi_device_handle_t handle, uint8_t *data, size_t len)
{
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = NULL,
        .rx_buffer = data,
    };
    esp_err_t ret = spi_device_transmit(handle, &t);
    return (ret == ESP_OK) ? len : -1;
}

void spi_add_device(spi_device_handle_t handle)
{
    if (opened_spi_count < SPI_PORTS_MAX) {
        opened_spi_handles[opened_spi_count++] = handle;
    }
}

void spi_timer_callback(void* arg)
{
    for (int i = 0; i < opened_spi_count; ++i) {
        spi_device_handle_t handle = opened_spi_handles[i];
        uint8_t data[SPI_BUF_SIZE] = {0};
        int len = spi_read(handle, data, SPI_BUF_SIZE);
        if (len > 0 && spi_content_count < SPI_CONTENT_BUFFER_SIZE) {
            memcpy(spi_content_buffer[spi_write_index].data, data, len);
            spi_content_buffer[spi_write_index].handle = handle;
            spi_content_buffer[spi_write_index].length = len;
            spi_write_index = (spi_write_index + 1) % SPI_CONTENT_BUFFER_SIZE;
            spi_content_count++;
        }
    }
}

void spi_timer_service_start(void)
{
    const esp_timer_create_args_t timer_args = {
        .callback = &spi_timer_callback,
        .name = "spi_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &spi_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(spi_timer, 50000)); // 50ms
}

spi_content_t* spi_read_buffer(void)
{
    if (spi_content_count <= 0) {
        return NULL;
    }
    spi_content_t* content = &spi_content_buffer[spi_read_index];
    spi_read_index = (spi_read_index + 1) % SPI_CONTENT_BUFFER_SIZE;
    spi_content_count--;
    return content;
}
