/* Isabella Kuhl, Laura Reeves, Joesph Roosi
* Main control program for controlling and initializing crawler
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "crawler_control.h"

void crawler_control_init()
{
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(STEERING_PWM_UNIT, STEERING_PWM_TIMER, &pwm_config);
    mcpwm_init(ESC_PWM_UNIT, ESC_PWM_TIMER, &pwm_config);

    mcpwm_gpio_init(STEERING_PWM_UNIT, STEERING_PWM_PIN, STEERING_PWM_GPIO);
    mcpwm_gpio_init(ESC_PWM_UNIT, ESC_PWM_PIN, ESC_PWM_GPIO);

    vTaskDelay(500/portTICK_PERIOD_MS);
}

void crawler_calibrate()
{
    printf("Calibrating...\n");
    vTaskDelay(3000 / portTICK_PERIOD_MS);  // Give yourself time to turn on crawler
    printf("High\n");
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, PWM_HIGH_US); // HIGH signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Low\n");
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, PWM_LOW_US);  // LOW signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Neutral\n");
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, PWM_NEUTRAL_US); // NEUTRAL signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, PWM_NEUTRAL_US); // reset the ESC to neutral (non-moving) value
}

static uint32_t esc_pwm_us = PWM_NEUTRAL_US;
static uint32_t steer_pwm_us = PWM_NEUTRAL_US;

static uint32_t esc_pwm_setpoint = PWM_NEUTRAL_US;
static uint32_t steer_pwm_setpoint = PWM_NEUTRAL_US;

uint32_t crawler_esc_get_value()
{
    return esc_pwm_us;
}

void crawler_esc_set_value(uint32_t pwm)
{
    if (pwm > PWM_HIGH_US || pwm < PWM_LOW_US){
        return;
    }
    if (pwm > PWM_HIGH_US) {
        esc_pwm_setpoint = PWM_HIGH_US;
    }
    else if (pwm < PWM_LOW_US) {
        esc_pwm_setpoint = PWM_LOW_US;
    }
    else {
        esc_pwm_setpoint = pwm;
    }
    esc_pwm_setpoint = pwm;
}

uint32_t crawler_steering_get_value()
{
    return steer_pwm_us;
}

void crawler_steering_set_value(uint32_t pwm)
{
    if (pwm > PWM_HIGH_US) {
        steer_pwm_setpoint = PWM_HIGH_US;
    }
    else if (pwm < PWM_LOW_US) {
        steer_pwm_setpoint = PWM_LOW_US;
    }
    else {
        steer_pwm_setpoint = pwm;
    }
}

int8_t crawler_get_direction()
{
    if (esc_pwm_us > PWM_NEUTRAL_US) {
        return -1;
    }
    else {
        return 1;
    }
}

#define max(a, b) (a) < (b) ? (b) : (a)
#define min(a, b) (a) > (b) ? (b) : (a)
static TaskHandle_t crawler_control_task_handle = NULL;
static void crawler_control_task()
{
    int esc_delta = 50;
    int steer_delta = 100;
    while (1)
    {
        if (esc_pwm_us != esc_pwm_setpoint) {
            esc_pwm_us = esc_pwm_setpoint < esc_pwm_us ?
                                max(esc_pwm_us - esc_delta, esc_pwm_setpoint) :
                                min(esc_pwm_us + esc_delta, esc_pwm_setpoint);

            mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER,
                                MCPWM_OPR_A, esc_pwm_us);
        }

        if (steer_pwm_us != steer_pwm_setpoint) {
            steer_pwm_us = steer_pwm_setpoint < steer_pwm_us ?
                                max(steer_pwm_us - steer_delta, steer_pwm_setpoint) :
                                min(steer_pwm_us + steer_delta, steer_pwm_setpoint);

            mcpwm_set_duty_in_us(STEERING_PWM_UNIT, STEERING_PWM_TIMER,
                                MCPWM_OPR_A, steer_pwm_us);
        }

        vTaskDelay(60/portTICK_PERIOD_MS);
    }
}

void crawler_control_start()
{
    if (crawler_control_task_handle != NULL) return;

    xTaskCreate(crawler_control_task,
        "crawler_control_task",
        4096,
        NULL, configMAX_PRIORITIES-1, &crawler_control_task_handle);
}

void crawler_control_stop()
{
    if (crawler_control_task_handle != NULL) {
        vTaskDelete(crawler_control_task_handle);
        crawler_control_task_handle = NULL;
    }
}