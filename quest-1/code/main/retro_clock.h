#ifndef __EC444T15_RETRO_CLOCK__
#define __EC444T15_RETRO_CLOCK__

#include <stdint.h>
#include <stdio.h>

typedef enum {
    RC_ALARM_STATE_NOT_ENABLED,
    RC_ALARM_STATE_ENABLED
} retro_clock_alarm_state_t;


typedef enum {
    RC_MODE_NOT_SET,
    RC_MODE_CLOCK,
    RC_MODE_SET_TIME,
    RC_MODE_SET_ALARM,
    RC_MODE_IN_ALARM
} retro_clock_mode_t;


typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} retro_clock_time_t;


// Private API
struct clock_internals;
typedef struct clock_internals clock_internals_t;

typedef struct {
    retro_clock_alarm_state_t alarm_state;
    retro_clock_mode_t clock_mode;
    retro_clock_time_t clock_time;
    retro_clock_time_t alarm_time;
    clock_internals_t *internals;
} retro_clock_t;

typedef int clock_update_handle;
typedef void (*clock_update_callback)(retro_clock_t *clock);


void retro_clock_init(retro_clock_t *clock);
void retro_clock_change_mode(retro_clock_t *clock, retro_clock_mode_t mode);

clock_update_handle
retro_clock_register_update_callback(retro_clock_t *clock, clock_update_callback callback);

// Time functions
void retro_clock_set_time(retro_clock_t *clock, retro_clock_time_t new_time);
void retro_clock_start(retro_clock_t *clock);
void retro_clock_stop(retro_clock_t *clock);

// Alarm functions
void retro_clock_alarm_set_state(retro_clock_t *clock, retro_clock_alarm_state_t alarm_state);
void retro_clock_alarm_set_time(retro_clock_t *clock, retro_clock_time_t new_time);
void retro_clock_alarm_dismiss(retro_clock_t *clock);

// Display Functions
void retro_clock_display_init();
void retro_clock_display_update(retro_clock_t *clock);

// Clock Hands Functions
void retro_clock_hands_init(retro_clock_t *clock);
void retro_clock_hands_update(retro_clock_t *clock);

// I/O function
void retro_clock_io_init(retro_clock_t *clock);
void retro_clock_io_update(retro_clock_t *clock);
void retro_clock_io_main(retro_clock_t *clock);

#endif

