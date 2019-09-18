#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "retro_clock.h"

static retro_clock_t g_clock;

void example_sound_alarm()
{
    retro_clock_time_t time = {
        .hours=23,
        .minutes=59,
        .seconds=55
    };

    retro_clock_time_t alarm_time = {
        .hours=0,
        .minutes=0,
        .seconds=0
    };

    retro_clock_set_time(&g_clock, time);
    retro_clock_change_mode(&g_clock, RC_MODE_CLOCK);
}

void app_main(void)
{
    retro_clock_init(&g_clock);

    retro_clock_display_init();
    retro_clock_hands_init(&g_clock);
    retro_clock_io_init(&g_clock);

    //retro_clock_register_update_callback(&g_clock, retro_clock_hands_update);
    retro_clock_register_update_callback(&g_clock, retro_clock_display_update);
  //  retro_clock_register_update_callback(&g_clock, retro_clock_io_update);

    example_sound_alarm();
}
