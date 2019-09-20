#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "alphanumeric_display.h"
#include "console_io.h"
#include <string.h>

#define LINE_BUFFER_SIZE (1024)

void app_main()
{
    io_uart_setup(LINE_BUFFER_SIZE);
	alphadisplay_init();
	alphadisplay_start();

    char digits[4] = {' ', ' ', ' ', ' '};
    io_uart_writeline("Start typing");

	while (1)
	{
        alphadisplay_write_ascii(0, digits[0]);
        alphadisplay_write_ascii(1, digits[1]);
        alphadisplay_write_ascii(2, digits[2]);
        alphadisplay_write_ascii(3, digits[3]);

        char letter = io_uart_getc();

        // Lets restrict this to upper case ltters
        // only (i don't like the way lower case looks
        // on the display.
        if (letter >= 97 && letter <= 122) {
            letter -= 32;
        }

        digits[0] = digits[1];
        digits[1] = digits[2];
        digits[2] = digits[3];
        digits[3] = letter;
	}
}

