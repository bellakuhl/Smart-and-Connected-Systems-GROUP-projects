#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "retro_clock.h"

#define MAX_UPDATE_CALLBACKS 4

struct clock_internals 
{
    clock_update_callback update_cb[MAX_UPDATE_CALLBACKS];
    int update_cb_count;
    xTaskHandle task_call_updates;
    xTaskHandle task_tick_clock;
}; 

static void retro_clock_tick(void *clk) 
{
    while (1) 
    {
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

        if (clock->alarm_state == RC_ALARM_STATE_ENABLED) {
            bool is_hour = clock->clock_time.hours == clock->alarm_time.hours;
            bool is_minute = clock->clock_time.minutes == clock->alarm_time.minutes;
            bool is_second = clock->clock_time.seconds == clock->alarm_time.seconds;

            if (is_hour && is_minute && is_second) {
                retro_clock_change_mode(clock, RC_MODE_IN_ALARM);
            }
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

static void retro_clock_notify_update(retro_clock_t *clock)
{
    for (int i = 0; i < clock->internals->update_cb_count; i++) {
        clock->internals->update_cb[i](clock);
    }
}

static void task_retro_clock_update(void *clk)
{
    retro_clock_t *clock = (retro_clock_t *)clk;
    while (1) 
    {
        retro_clock_notify_update(clock);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}


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
    clock->internals->task_call_updates = NULL;
    clock->internals->task_tick_clock = NULL;
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
    retro_clock_notify_update(clock);
}


void retro_clock_set_time(retro_clock_t *clock, retro_clock_time_t new_time)
{
    clock->clock_time.hours = new_time.hours;
    clock->clock_time.minutes = new_time.minutes;
    clock->clock_time.seconds = new_time.seconds;

    retro_clock_notify_update(clock);
}


void retro_clock_start(retro_clock_t *clock) 
{
    if (clock->internals->task_call_updates != NULL) {
        return;
    }

    xTaskCreate(
        retro_clock_tick, 
        "clock_tick", 
        4096,
        (void *)clock,
        configMAX_PRIORITIES,
        &clock->internals->task_tick_clock
    );

    xTaskCreate(
        task_retro_clock_update, 
        "clock_update", 
        4096,
        (void *)clock,
        configMAX_PRIORITIES - 3,
        &clock->internals->task_call_updates
    );
}


void retro_clock_stop(retro_clock_t *clock)
{
    if (clock->internals->task_call_updates == NULL) {
        return;
    }

    vTaskDelete(clock->internals->task_tick_clock);
    vTaskDelete(clock->internals->task_call_updates);
    clock->internals->task_call_updates = NULL;
    clock->internals->task_tick_clock = NULL;
}


void retro_clock_alarm_set_state(retro_clock_t *clock, retro_clock_alarm_state_t alarm_state)
{
    if (clock->alarm_state == alarm_state) {
        // Already done, do nothing
        return;
    }

    clock->alarm_state = alarm_state;
    retro_clock_notify_update(clock);
}


void retro_clock_alarm_set_time(retro_clock_t *clock, retro_clock_time_t new_time)
{
    clock->alarm_time.hours = new_time.hours;
    clock->alarm_time.minutes = new_time.minutes;
    clock->alarm_time.seconds = new_time.seconds;
    retro_clock_notify_update(clock);
}


void retro_clock_alarm_dismiss(retro_clock_t *clock)
{
    if (clock->clock_mode == RC_MODE_IN_ALARM) 
    {
        retro_clock_change_mode(clock, RC_MODE_CLOCK);
    }
}

