/* Isabella Kuhl, Laura Reeves, Joesph Roosi
* Header for crawler control
*/

#ifndef __T15_CRAWLER_CONTROL__
#define __T15_CRAWLER_CONTROL__

#define STEERING_PWM_UNIT   MCPWM_UNIT_0
#define STEERING_PWM_TIMER  MCPWM_TIMER_0
#define STEERING_PWM_PIN    MCPWM0A
#define STEERING_PWM_GPIO   GPIO_NUM_15

#define ESC_PWM_UNIT   MCPWM_UNIT_0
#define ESC_PWM_TIMER  MCPWM_TIMER_1
#define ESC_PWM_PIN    MCPWM1A
#define ESC_PWM_GPIO   GPIO_NUM_12

#define PWM_LOW_US 900
#define PWM_HIGH_US 2400
#define PWM_NEUTRAL_US 1500

void crawler_control_init();
void crawler_calibrate();

uint32_t crawler_esc_get_value();
void crawler_esc_set_value(uint32_t pwm);

uint32_t crawler_steering_get_value();
void crawler_steering_set_value(uint32_t pwm);

int8_t crawler_get_direction();

#endif
