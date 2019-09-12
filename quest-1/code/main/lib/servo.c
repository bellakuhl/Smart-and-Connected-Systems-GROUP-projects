#include <stdint.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "soc/mcpwm_periph.h"

#include "servo.h"


static void servo_update_position(servo_config_t *config)
{
    uint32_t pw = config->current_pulse_width;
    mcpwm_set_duty_in_us(config->unit, config->timer, MCPWM_OPR_A, pw);
}


void servo_init(servo_config_t *config, int32_t start_angle)
{

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(config->unit, config->timer, &pwm_config);
    mcpwm_gpio_init(config->unit, config->output_pin, config->gpio_pin);

    servo_set_angle_degrees(config, start_angle);
    vTaskDelay(500/portTICK_PERIOD_MS);
}


int32_t servo_get_angle_degrees(servo_config_t *config)
{
    float pct = (float)(config->current_pulse_width - config->min_pulse_width_us)/
                (float)(config->max_pulse_width_us - config->min_pulse_width_us);

    uint32_t range = config->max_angle_degrees - config->min_angle_degrees;
    int32_t angle = roundf(config->min_angle_degrees + (pct * range));
    //printf("Get Angle - Pct: %.3f, Range: %u, Angle: %u\n", pct, range, angle);
    return angle;
}


void servo_set_angle_degrees(servo_config_t *config, int32_t degrees)
{
    if (degrees > config->max_angle_degrees || degrees < config->min_angle_degrees)
    {
        // out of range, do nothing.
        return;
    }

    float pct = (float)(degrees - config->min_angle_degrees)/
                (float)(config->max_angle_degrees - config->min_angle_degrees);

    uint32_t range = config->max_pulse_width_us - config->min_pulse_width_us;
    uint32_t pw = roundf(config->min_pulse_width_us + (pct * range));

    config->current_pulse_width = pw;
    servo_update_position(config);
    //printf("Set Angle - Angle: %d Pct: %.3f, Range: %u, PW: %u\n", degrees, pct, range, pw);
    //fflush(stdout);
    vTaskDelay(10);
}

