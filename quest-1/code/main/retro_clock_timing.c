#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "retro_clock.h"

#define HW_TIMER_GROUP TIMER_GROUP_0
#define HW_TIMER_IDX 0
#define HW_TIMER_CLK_DIVIDER 65536
#define HW_TIMER_TIME_SCALE (TIMER_BASE_CLK/HW_TIMER_CLK_DIVIDER)

#define MAX_UPDATE_CALLBACKS 4

struct clock_internals
{
    clock_update_callback update_cb[MAX_UPDATE_CALLBACKS];
    int update_cb_count;

    xTaskHandle task_call_updates;
    xTaskHandle task_tick_clock;
};


static void timer_to_clock(uint64_t value, retro_clock_time_t *time)
{
    double seconds = ((double)value / (double)HW_TIMER_TIME_SCALE);
    uint8_t hr = (uint8_t)(seconds / 3600);
    uint8_t min = (uint8_t)((seconds - (3600*hr))/60);
    uint8_t sec = (uint8_t)(seconds - (3600*hr) - (60*min));

    time->hours = hr;
    time->minutes = min;
    time->seconds = sec;
}


static uint64_t clock_to_timer(retro_clock_time_t* time)
{
    double seconds = time->hours*3600 + time->minutes*60 + time->seconds;
    return (uint64_t)(seconds * HW_TIMER_TIME_SCALE);
}


static void retro_clock_tick(void *clk)
{
    retro_clock_t *clock = (retro_clock_t *)clk;

    while (1)
    {
        uint64_t value;
        timer_get_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, &value);
        timer_to_clock(value, &(clock->clock_time));
        vTaskDelay(500/portTICK_PERIOD_MS);
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

    timer_config_t hw_timer_config = {
        .alarm_en = 0,
        .counter_en = 0,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = false,
        .divider = HW_TIMER_CLK_DIVIDER
    };

    timer_init(HW_TIMER_GROUP, HW_TIMER_IDX, &hw_timer_config);
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

    uint64_t counter_val = clock_to_timer(&(clock->clock_time));
    timer_set_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, counter_val);
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

    timer_start(HW_TIMER_GROUP, HW_TIMER_IDX);
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

    timer_pause(HW_TIMER_GROUP, HW_TIMER_IDX);
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

