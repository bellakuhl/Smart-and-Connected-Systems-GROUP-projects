#include <stdarg.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_vfs_dev.h"
#include "console_io.h"

#define IO_UART_TX  (GPIO_NUM_1)
#define IO_UART_RX  (GPIO_NUM_3)

static uint8_t *io_uart_input_g = NULL;

static short input_filter_numerals(char letter)
{
    if (letter < 0x30 || letter > 0x39) {
        return 0;
    }

    return 1;
}


int io_uart_setup(size_t line_buffer_size)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };


    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(
        UART_NUM_1,
        IO_UART_TX,
        IO_UART_RX,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );

    // TODO: Return non-zero status
    // if setting up the uart driver
    // fails.
    ESP_ERROR_CHECK(
        uart_driver_install(UART_NUM_1, line_buffer_size * 2, 0, 0, NULL, 0)
    );

    esp_vfs_dev_uart_use_driver(UART_NUM_1);
    io_uart_input_g = malloc(sizeof(uint8_t) * line_buffer_size);

    if (io_uart_input_g == NULL) {
        return -1;
    }

    return 0;
}


char io_uart_getc()
{
    while(1) {
        int len = uart_read_bytes(UART_NUM_1, io_uart_input_g, 1, 20 / portTICK_RATE_MS);
        if (len == 1) { return io_uart_input_g[0]; }
    }
}


size_t io_uart_readline(char *line_buffer, size_t size, io_input_filter filterFn, bool echo)
{
    memset(line_buffer, '\0', sizeof(char) * size);

    size_t line_buffer_char_count = 0;
    while (1) {
        int len = uart_read_bytes(UART_NUM_1, io_uart_input_g, size, 20 / portTICK_RATE_MS);

        for (int pos = 0; pos < len; pos++)
        {
            const char letter = (const char)(io_uart_input_g[pos]);
            if (letter == '\r')
            {
                // Input contains a newline, only consider input
                // until the newline, and then print the hex value.
                line_buffer[line_buffer_char_count] = '\0';
                int count  = line_buffer_char_count;
                line_buffer_char_count = 0;
                return count;
            }
            else if (line_buffer_char_count < size) {
                if (filterFn == NULL || filterFn(letter))
                {
                    line_buffer[line_buffer_char_count] = letter;
                    line_buffer_char_count += 1;
                    if (echo) {
                        io_uart_uwritec(letter);
                    }
                }
            }
        }

        memset(io_uart_input_g, 0, len);
    }

}

uint32_t io_uart_read_uint32()
{
    char line[1024];
    memset(line, '\0', 1024);
    io_uart_readline(line, 1024, &input_filter_numerals,true);

    uint32_t as_int = strtoul(line, NULL, 10);
    // When strutoull overflows it returns UINT64_MAX
    // and sets errno to ERANGE
    if (as_int == UINT32_MAX && errno == ERANGE) {
        io_uart_writes("\r\nOverflow");
        // clear errno to express the error has been
        // handled. If the user does indeed enter UINT64_MAX
        // as a legitimate input after overflowing, this
        // would erroneously output 'Overflow'
        errno = 0;
        return 0;
    }

    return as_int;
}


void io_uart_uwritec(const char c)
{
    uart_write_bytes(UART_NUM_1, &c, sizeof(char));
}


void io_uart_writes(const char *line)
{
    if (line == NULL) return;
    uart_write_bytes(UART_NUM_1, line, strlen(line));
}


void io_uart_writef(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    io_uart_writes((const char*)&buffer);
}


void io_uart_writeline(const char *line)
{
    io_uart_writes(line);
    io_uart_writes("\r\n");
}


void io_uart_cleanup()
{
    if (io_uart_input_g != NULL) {
        free(io_uart_input_g);
    }
    // TODO: Uninstall the UART driver?
}


