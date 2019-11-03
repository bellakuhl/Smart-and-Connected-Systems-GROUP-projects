#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "crawler.h"

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

void crawler_esc_set_value(uint32_t pwm)
{
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER,
            MCPWM_OPR_A, pwm);
}

void crawler_steering_set_value(uint32_t pwm)
{
    mcpwm_set_duty_in_us(STEERING_PWM_UNIT, STEERING_PWM_TIMER,
            MCPWM_OPR_A, pwm);
}

