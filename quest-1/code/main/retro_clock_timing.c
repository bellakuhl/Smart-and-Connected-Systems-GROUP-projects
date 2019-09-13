#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/timer.h"
#include "retro_clock.h"


#include <stdio.h>
#define MAX_UPDATE_CALLBACKS 3
//#define TIMER_DIVIDER 16
//#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)

struct clock_internals 
{
    clock_update_callback update_cb[MAX_UPDATE_CALLBACKS];
    int update_cb_count;

    /*
    timer_group_t timer_group;
    timer_idx_t timer_index;
    timer_config_t timer_config;
    */

    xTaskHandle update_task;
}; 


static void retro_clock_update(void *clk)
{
    while (1) {
        retro_clock_t *clock = (retro_clock_t *)clk;

        clock->clock_time.seconds += 1;

        if (clock->clock_time.seconds >= 60) {
            clock->clock_time.seconds -= 60;
            clock->clock_time.minutes += 1;
        }

        if (clock->clock_time.minutes >= 60) {
            clock->clock_time.minutes -= 60;
            clock->clock_time.hours += 1;
        }

        if (clock->clock_time.hours >= 24) {
            clock->clock_time.hours = 0;
        }

        fflush(stdout);
        vTaskDelay(1000/portTICK_PERIOD_MS);

        for (int i = 0; i < clock->internals->update_cb_count; i++) {
            clock->internals->update_cb[i](clock);
        }
    }
}


/*
static void IRAM_ATTR retro_clock_isr(void *clk) 
{
    retro_clock_t *clock = (retro_clock_t *)clk;

    double seconds;

    timer_get_counter_time_sec(
        clock->internals->timer_group,
        clock->internals->timer_index,
        &seconds);

    if (seconds >= 86400) 
    {
        seconds = 0;
        timer_set_counter_value(
            clock->internals->timer_group,
            clock->internals->timer_index,
            0x0LL);
    }

    uint8_t hours = (uint8_t)(seconds / 3600);
    uint8_t minutes = (uint8_t)(seconds - (3600 * hours))/60;
    uint8_t sec = (uint8_t)(seconds - (3600 * hours) - (60 * minutes));

    clock->clock_time.hours = hours;
    clock->clock_time.minutes = minutes;
    clock->clock_time.seconds = sec; 
}
*/


void retro_clock_init(retro_clock_t *clock) 
{
    clock->alarm_state = RC_ALARM_STATE_NOT_ENABLED;

    clock->clock_mode = RC_MODE_NOT_SET;
    clock->clock_time.hours = 0;
    clock->clock_time.minutes = 0;
    clock->clock_time.seconds = 0;

    clock->alarm_time.hours = 0;
    clock->alarm_time.minutes = 0;
    clock->alarm_time.seconds = 0;

    clock->internals = (clock_internals_t*)malloc(sizeof(clock_internals_t));
    if (clock->internals == NULL) {
        // TODO: Handle malloc failure 
    }

    clock->internals->update_cb_count = 0;
    clock->internals->update_task = NULL;
/*
    timer_config_t timer_config = {
        .alarm_en = true,
        .counter_en = TIMER_PAUSE, 
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = true,
        .divider = TIMER_DIVIDER
    };

    clock->internals->timer_group = TIMER_GROUP_0;
    clock->internals->timer_index = 0;
    clock->internals->timer_config = timer_config;

    timer_init(
        clock->internals->timer_group, 
        clock->internals->timer_index, 
        &timer_config);

    timer_set_counter_value(
        clock->internals->timer_group,
        clock->internals->timer_index,
        0x0LL
    );

    timer_set_alarm_value(
        clock->internals->timer_group,
        clock->internals->timer_index,
        1 * TIMER_SCALE // 1 second counting
    );

    timer_enable_intr(
        clock->internals->timer_group,
        clock->internals->timer_index);

    timer_isr_register(
        clock->internals->timer_group,
        clock->internals->timer_index,
        retro_clock_isr,
        (void *) clock, 
        ESP_INTR_FLAG_IRAM, 
        NULL);
    */
}


clock_update_handle retro_clock_register_update_callback(
    retro_clock_t *clock, 
    clock_update_callback function
) 
{
    if (clock->internals->update_cb_count >= MAX_UPDATE_CALLBACKS) {
        return -1; 
    }

    clock->internals
         ->update_cb[clock->internals->update_cb_count++] = function;

    return clock->internals->update_cb_count;
}


void retro_clock_change_mode(retro_clock_t *clock, retro_clock_mode_t mode)
{
    if (clock->clock_mode == mode) {
        return; // nothing to do.
    }

    switch (mode) 
    {
        case RC_MODE_CLOCK:
            retro_clock_start(clock);
            break;

        case RC_MODE_SET_TIME:
            retro_clock_stop(clock);
            break;

        case RC_MODE_SET_ALARM:
            break;

        case RC_MODE_IN_ALARM:
            break;
        
        default:
            break;
    }

    clock->clock_mode = mode;
}

void retro_clock_set_time(retro_clock_t *clock, retro_clock_time_t new_time)
{
    clock->clock_time.hours = new_time.hours;
    clock->clock_time.minutes = new_time.minutes;
    clock->clock_time.seconds = new_time.seconds;
}


void retro_clock_start(retro_clock_t *clock) 
{
    if (clock->internals->update_task != NULL) {
        return;
    }

    /*
    timer_start(
        clock->internals->timer_group, 
        clock->internals->timer_index);
    */

    xTaskCreate(
        retro_clock_update, 
        "clock_update", 
        4096,
        (void *)clock,
        configMAX_PRIORITIES - 3,
        &clock->internals->update_task
    );
}


void retro_clock_stop(retro_clock_t *clock)
{
    if (clock->internals->update_task == NULL) {
        return;
    }
    
    /*
        timer_pause(
            clock->internals->timer_group, 
            clock->internals->timer_index);
    */

    vTaskDelete(clock->internals->update_task);
    clock->internals->update_task = NULL;
}


void retro_clock_alarm_set_state(retro_clock_t *clock, retro_clock_alarm_state_t alarm_state)
{
    if (clock->alarm_state == alarm_state) {
        // Already done, do nothing
        return;
    }

    if (alarm_state == RC_ALARM_STATE_NOT_ENABLED) {
        // TODO: Disable the alarm
    }

    if (alarm_state == RC_ALARM_STATE_ENABLED) {
        // TODO: Enable the alarm
    }

    clock->alarm_state = alarm_state;
}


void retro_clock_alarm_set_time(retro_clock_t *clock, retro_clock_time_t new_time)
{
    clock->clock_time.hours = new_time.hours;
    clock->clock_time.minutes = new_time.minutes;
    clock->clock_time.seconds = new_time.seconds;

    if (clock->alarm_state == RC_ALARM_STATE_ENABLED) {
        // TODO: Update timer if enabled
    }
}


void retro_clock_alarm_dismiss(retro_clock_t *clock)
{
    if (clock->clock_mode == RC_MODE_IN_ALARM) 
    {
        retro_clock_change_mode(clock, RC_MODE_CLOCK);
    }
}

