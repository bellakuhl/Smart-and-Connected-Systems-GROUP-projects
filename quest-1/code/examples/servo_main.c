#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "servo.h"

void random_servo_task(void *arg)
{
    servo_config_t servo;

    servo.unit = MCPWM_UNIT_0;
    servo.timer = MCPWM_TIMER_1;
    servo.output_pin = MCPWM1A;
    servo.gpio_pin = GPIO_NUM_15;

    servo.min_pulse_width_us = 1000;
    servo.max_pulse_width_us = 2600;
    servo.min_angle_degrees = 0;
    servo.max_angle_degrees = 180;
    servo_init(&servo, 0);


    while (1)
    {
        int number = rand();
        float pct = (float)number / (float)RAND_MAX;

        int32_t angle = servo.min_angle_degrees +
                        (pct * (servo.max_angle_degrees - servo.min_angle_degrees));

        servo_set_angle_degrees(&servo, angle);
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

void sweep_servo_task(void *arg)
{
    servo_config_t servo;

    servo.unit = MCPWM_UNIT_0;
    servo.timer = MCPWM_TIMER_0;
    servo.output_pin = MCPWM0A;
    servo.gpio_pin = GPIO_NUM_12;

    servo.min_pulse_width_us = 1000;
    servo.max_pulse_width_us = 2600;
    servo.min_angle_degrees = 0;
    servo.max_angle_degrees = 180;
    servo_init(&servo, 0);

    int32_t direction = 1;
    while (1)
    {
        int32_t angle = servo_get_angle_degrees(&servo) + direction;
        if (angle <= servo.min_angle_degrees) direction = 1;
        if (angle >= servo.max_angle_degrees) direction = -1;
        servo_set_angle_degrees(&servo, angle);
    }
}

void app_main(void)
{
    xTaskCreate(sweep_servo_task, "sweep_servo_task", 4096, NULL, 5, NULL);
    xTaskCreate(random_servo_task, "random_servo_task", 4096, NULL, 5, NULL);
}

