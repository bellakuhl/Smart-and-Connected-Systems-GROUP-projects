#include "retro_clock.h"
#include "lib/servo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

servo_config_t minutes_hand;
servo_config_t seconds_hand;
void retro_clock_hands_init(retro_clock_t *clock)
{

    minutes_hand.unit = MCPWM_UNIT_0;
    minutes_hand.timer = MCPWM_TIMER_0;
    minutes_hand.output_pin = MCPWM0A;
    minutes_hand.gpio_pin = GPIO_NUM_12;

    minutes_hand.min_pulse_width_us = 1000;
    minutes_hand.max_pulse_width_us = 2000;
    minutes_hand.min_angle_degrees = 0;
    minutes_hand.max_angle_degrees = 59;
    servo_init(&minutes_hand, 0);


    seconds_hand.unit = MCPWM_UNIT_0;
    seconds_hand.timer = MCPWM_TIMER_1;
    seconds_hand.output_pin = MCPWM1A;
    seconds_hand.gpio_pin = GPIO_NUM_27;

    seconds_hand.min_pulse_width_us = 1000;
    seconds_hand.max_pulse_width_us = 2000;
    seconds_hand.min_angle_degrees = 0;
    seconds_hand.max_angle_degrees = 59;
    servo_init(&seconds_hand, 0);
}


void retro_clock_hands_update(retro_clock_t *clock)
{
    retro_clock_time_t time;
    if (clock->clock_mode == RC_MODE_SET_ALARM) {
        time = clock->alarm_time;
    }
    else {
        time = clock->clock_time;
    }

    servo_set_angle_degrees(&minutes_hand, 59 - time.minutes);
    servo_set_angle_degrees(&seconds_hand, 59 - time.seconds);
    vTaskDelay(300/portTICK_PERIOD_MS);
}

