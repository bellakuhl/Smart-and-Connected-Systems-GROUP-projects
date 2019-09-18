#include "retro_clock.h"
#include "lib/alphanumeric_display.h"

#include <stdio.h>

char numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

void retro_clock_display_init()
{
    alphadisplay_init();
    alphadisplay_start();
}


void retro_clock_display_update(retro_clock_t *clock)
{
    uint8_t hr0 = clock->clock_time.hours / 10;
    uint8_t hr1 = clock->clock_time.hours % 10;
    uint8_t min0 = clock->clock_time.minutes / 10;
    uint8_t min1 = clock->clock_time.minutes % 10;

    alphadisplay_write_ascii(0, numbers[hr0]);
    alphadisplay_write_ascii(1, numbers[hr1]);
    alphadisplay_write_ascii(2, numbers[min0]);
    alphadisplay_write_ascii(3, numbers[min1]);
}


void retro_clock_display_fatal_error(char *message)
{
}
