#include "retro_clock.h"
#include "lib/alphanumeric_display.h"

char numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};


void retro_clock_display_init()
{
    alphadisplay_init();
}


void retro_clock_display_update(retro_clock_t *clock)
{
    if (clock->clock_mode == RC_MODE_CLOCK) {
        alphadisplay_set_blink(ALPHADISPLAY_BLINK_OFF);
    }
    else {
        alphadisplay_set_blink(ALPHADISPLAY_BLINK_1HZ);
    }

    if (clock->clock_mode == RC_MODE_NOT_SET) {
        alphadisplay_write_ascii(0, '-');
        alphadisplay_write_ascii(1, '-');
        alphadisplay_write_ascii(2, '-');
        alphadisplay_write_ascii(3, '-');
    }
    else if (clock->clock_mode == RC_MODE_IN_ALARM) {
        alphadisplay_write_ascii(0, 'A');
        alphadisplay_write_ascii(1, 'L');
        alphadisplay_write_ascii(2, 'R');
        alphadisplay_write_ascii(3, 'M');
    }
    else {
        retro_clock_time_t time;
        if (clock->clock_mode == RC_MODE_SET_ALARM) {
            time = clock->alarm_time;
        }
        else {
            time = clock->clock_time;
        }


        uint8_t hr0 = time.hours / 10;
        uint8_t hr1 = time.hours % 10;
        uint8_t min0 = time.minutes / 10;
        uint8_t min1 = time.minutes % 10;

        alphadisplay_write_ascii(0, numbers[hr0]);
        alphadisplay_write_ascii(1, numbers[hr1]);
        alphadisplay_write_ascii(2, numbers[min0]);
        alphadisplay_write_ascii(3, numbers[min1]);
    }
}

