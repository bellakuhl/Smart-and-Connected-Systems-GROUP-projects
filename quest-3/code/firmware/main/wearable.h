// Joseph Rossi, Isabella Kuhl, Laura Reeve
#ifndef __TEAM15_WEARABLE__
#define __TEAM15_WEARABLE__

#include <stdint.h>

#define HOME_STATION_IP "192.168.1.108"

typedef enum {
    WEARABLE_SENSOR_STEP = 0,
    WEARABLE_SENSOR_TEMPERATURE,
    WEARABLE_SENSOR_BATTERY,
    WEARABLE_NUM_SENSORS
} wearable_sensor_t;


typedef enum {
    SENSOR_DISABLED = 0,
    SENSOR_ENABLED
} wearable_sensor_en_t;


typedef enum {
    WEARABLE_ALERT_DRINK_WATER,
    WEARABLE_NUM_ALERTS
} wearable_alert_t;


typedef struct {
    /* The current body temperature in degc. If disabled, this should be -1 */
    float body_temperature_degc;

    /* The current batter level in volts. If disabled, this should be -1 */
    float battery_level_volts;

    /* The number of steps recorded. If steps is disabled, this should be -1 */
    int32_t steps;
} wearable_sensor_reading_t;


typedef struct {
    int8_t battery_sensor_enabled;
    int8_t temperature_sensor_enabled;
    int8_t step_sensor_enabled;
    uint8_t alert_now;
    uint32_t alert_period_sec;
} wearable_settings_t;


/**
 * When called this funciton should initialize all sensors (configure GPIOs, PWMs, etc)
 * so they are ready to be read via `wearable_sensors_read`
 */
void wearable_sensors_init();


/*
 * When called, the `reading` argument should be populated with
 * the latest reading from the appropriate sensors.
 *
 * Any value for a sensor that is disabled should be -1.
 */
int wearable_sensors_read(wearable_sensor_reading_t *reading);


/*
 * Returns whether or not `sensor` is enabled and recording data.
 */
wearable_sensor_en_t wearable_sensor_get_enable(wearable_sensor_t sensor);

/*
 * Enable or disable `sensor` from recording values.
 */
void wearable_sensor_set_enable(wearable_sensor_t sensor, wearable_sensor_en_t en_state);


/*
 * Schedules regularly
 *
 * To disable alerts,  pass `period_sec` as 0.
 */
void wearable_schedule_alert(uint32_t period_sec);


/*
 * Used to trigger an alert immediately.
 */
void wearable_trigger_alert();

#endif

