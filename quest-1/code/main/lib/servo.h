#ifndef __EC444_SERVO___
#define __EC444_SERVO___

#include <stdint.h>
#include "driver/mcpwm.h"

typedef struct
{
    uint16_t        gpio_pin;
    uint32_t        min_pulse_width_us;
    uint32_t        max_pulse_width_us;
    int32_t         min_angle_degrees;
    int32_t         max_angle_degrees;
    uint32_t        current_pulse_width;
    mcpwm_unit_t    unit;
    mcpwm_timer_t   timer;
    mcpwm_io_signals_t output_pin;

} servo_config_t;

/**
 * Initializes the mcpwm and moves servo to the starting angle.
 *
 * @param config The servo configuration (with parameters set!)
 * @param start_angle The starting angle (between min and max) to move
 *                    the sevo to.
 */
void servo_init(servo_config_t *config, int32_t start_angle);

/**
 * Get the current angle of the servo.
 */
int32_t servo_get_angle_degrees(servo_config_t *config);

/**
 * Set the angle of the servo within the min/max range set in the
 * config.
 */
void servo_set_angle_degrees(servo_config_t *config, int32_t degrees);

#endif

