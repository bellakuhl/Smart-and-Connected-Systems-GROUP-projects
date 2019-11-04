#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "pins.h"

#define UART_NUM UART_NUM_1
#define SIZE 6

static float inches_to_meters(float inches) {
    return inches * 0.0254;
}

void ultrasonic_serial_init()
{
    uart_config_t config = {
        .baud_rate              = 9600,
        .data_bits              = UART_DATA_8_BITS,
        .parity                 = UART_PARITY_DISABLE,
        .stop_bits              = UART_STOP_BITS_1,
        .flow_ctrl              = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh    = 0
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &config));
    ESP_ERROR_CHECK(uart_set_pin(
        UART_NUM,
        UART_PIN_NO_CHANGE,
        RX,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    ));

    uart_set_line_inverse(UART_NUM, UART_INVERSE_RXD);

    const int rx_buffer_size = 256;
    ESP_ERROR_CHECK(uart_driver_install(
                UART_NUM,
                rx_buffer_size,
                0, 0, NULL, 0));
}

float ultrasonic_read_latest()
{
    uart_flush_input(UART_NUM);
    uint8_t data[SIZE];
    uart_read_bytes(UART_NUM, data, SIZE, 100);

    char num[4];
    memcpy(num, &data[1], 3);
    num[3] = '\0';

    int32_t inches = atoi(num);
    return inches_to_meters(inches);
}

