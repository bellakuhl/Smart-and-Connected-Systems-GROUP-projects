#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "retro_clock.h"

void retro_clock_init(retro_clock_t *clock) 
{
    clock->clock_mode = RC_MODE_NOT_SET;
    clock->clock_time.hours = 0;
    clock->clock_time.minutes = 0;
    clock->clock_time.seconds = 0;

    clock->alarm_time.hours = 0;
    clock->alarm_time.minutes = 0;
    clock->alarm_time.seconds = 0;
}


void app_main(void)
{
    retro_clock_display_init();

    retro_clock_t *clock = (retro_clock_t *)malloc(sizeof(retro_clock_t));
    if (clock == NULL) {
        retro_clock_display_fatal_error("FAILED TO ALLOCATE MEMORY");
        return;
    }

    retro_clock_init(clock);
    retro_clock_hands_init(clock);
    retro_clock_io_init(clock);

    // TOOD: Define tasks and timers for running the clock
    // and changing the modes.
}

