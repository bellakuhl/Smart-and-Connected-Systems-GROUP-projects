#ifndef __EC444_CONSOLE_IO__
#define __EC444_CONSOLE_IO__

#include <stddef.h>
#include <stdbool.h>


/**
 * A function used to filter input of characters.
 * This function must return a truthy number (1 for example)
 * if a character is allowed, or 0 if it is not.
 */
typedef short (*io_input_filter)(char letter);


/**
 * Intall UART drivers and setup
 * line buffering.
 */
int io_uart_setup(size_t line_buffer_size);


/**
 * Get the next char of data received.
 *
 * Warning: This is a blocking function!
 *
 * @returns The first character received when
 *          this function is called.
 */
char io_uart_getc();


/**
 * Read a line of input
 *
 * Warning: This is a blocking function!
 *
 * @param line_buffer   The buffer to store user input
 * @param size          The size of the line buffer
 * @param filterFn      A function used to filter characters input. This
 *                      effectively blocks the user from entering unwanted
 *                      characters.
 * @param echo          Display user inputs back to the user.
 *
 * @returns The number of characters the user typed.
 */
size_t io_uart_readline(
        char *line_buffer,
        size_t size,
        io_input_filter filterFn,
        bool echo);


/**
 * Transmit a single byte over UART
 */
void io_uart_uwritec(const char c);


/**
 * Transmit string of bytes over UART
 */
void io_uart_writes(const char *line);


/**
 * Transmit a formatted string over UART.
 */
void io_uart_writef(const char *format, ...);


/**
 * Transmit string of bytes over UART and include
 * the newline.
 */
void io_uart_writeline(const char *line);


/**
 * Get an integer from the user.
 */
uint32_t io_uart_read_uint32();


/**
 * Free up any resources used UART.
 */
void io_uart_cleanup();


#endif

