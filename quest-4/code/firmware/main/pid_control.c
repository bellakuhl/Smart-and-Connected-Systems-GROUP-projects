/* Isabella Kuhl, Joseph Rossi, Laura Reeves
*  10/31/19
* This program implements PID control for speed control for the crawler
*/
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define DIVIDER 15000
#define TIMER_SCALE (TIMER_BASE_CLK/DIVIDER)
#define PID_TIMER (0.00002*TIMER_SCALE) // 0.1 seconds
#define HW_TIMER_GROUP TIMER_GROUP_0
#define HW_TIMER_IDX 0

// PID constants
#define KP 0.3
#define KD 1
#define KI 1

static float setpoint = 40;
// Flag for dt
static int dt_complete = 0;
// error for PID loop
static float previous_error = 0;
static float integral = 0;


// Define timer interrupt handler
void IRAM_ATTR timer_isr()
{
    // Indicate timer has fired
    dt_complete = 1;
    // Clear interrupt
    TIMERG0.int_clr_timers.t0 = 1;
}

// Set up periodic timer for dt = 100ms
static void periodic_timer_init()
{
    // Basic parameters of the timer
    timer_config_t config = {
        .alarm_en = 1,
        .counter_en = 0,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = true,
        .divider = DIVIDER
    };

    // register timer interrupt
    timer_init(HW_TIMER_GROUP, HW_TIMER_IDX, &config);

    // Timer's counter will initially start from value below
    timer_set_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, 0);

    // Configure the alarm value and the interrupt on alarm.
    timer_set_alarm_value(HW_TIMER_GROUP, HW_TIMER_IDX, PID_TIMER);
    timer_enable_intr(HW_TIMER_GROUP, HW_TIMER_IDX);
    timer_isr_register(HW_TIMER_GROUP, HW_TIMER_IDX, timer_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);

    // Start timer
    timer_start(HW_TIMER_GROUP, HW_TIMER_IDX);
}

float PID(float measured_value) {
  uint64_t timer_val;
  timer_get_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, &timer_val);

  double dt = ((double)timer_val / (double)TIMER_SCALE);
  float error = setpoint - measured_value;

  integral = integral + error * dt;

  float derivative = (error - previous_error) / dt;
  float output = KP * error; //+ KI * integral + KD * derivative;
  previous_error = error;
  printf("Setpoint: %.1f, Measurement: %.1f, Error: %.1f, Previous Error: %.1f\n",
          setpoint, measured_value, error, previous_error);

  return output;
}

void PID_init()
{
    periodic_timer_init();
}

void PID_set_setpoint(float sp)
{
    setpoint = sp;
}

