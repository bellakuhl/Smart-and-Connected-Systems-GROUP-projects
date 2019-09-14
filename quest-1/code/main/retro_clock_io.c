#include "retro_clock.h"
#include "lib/console_io.h"

void retro_clock_io_init(retro_clock_t *clock)
{
}

void retro_clock_io_update(retro_clock_t *clock)
{
    printf("Time: %02d:%02d:%02d\n",
        clock->clock_time.hours,
        clock->clock_time.minutes,
        clock->clock_time.seconds
    );

    if (clock->clock_mode == RC_MODE_IN_ALARM) {
        printf("Alarm is sounding!\n");
    }
}

void retro_clock_io_main(retro_clock_t *clock)
{
}

