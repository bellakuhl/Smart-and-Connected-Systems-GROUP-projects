#ifndef __TEAM15_WEARABLE__
#define __TEAM15_WEARABLE__

#define HOME_STATION_IP "192.168.1.108"

typedef enum {
    WEARABLE_SENSOR_STEP,
    WEARABLE_SENSOR_TEMPERATURE,
    WEARABLE_SENSOR_BATTERY,
    WEARABLE_NUM_SENSORS
} wearable_sensor_t;


typedef enum {
    SENSOR_DISABLED,
    SENSOR_ENABLED
} wearable_sensor_en_t;


typedef enum {
    WEARABLE_ALERT_DRINK_WATER
    WEARABLE_NUM_ALERTS
} wearable_alert_t;


typedef struct {
    /* The current body temperature in degc. If disabled, this should be -1 */
    float body_temperature_degc;

    /* The current batter level in volts. If disabled, this should be -1 */
    float batter_level_volts;

    /* The number of steps recorded. If steps is disabled, this should be -1 */
    int32_t steps;
} wearable_sensor_reading_t;


/**
 * When called this funciton should initialize all sensors (configure GPIOs, PWMs, etc)
 * so they are ready to be read via `wearable_biometrics_read`
 */
void wearable_biometrics_init_sensors();


/*
 * When called, the `reading` argument should be populated with
 * the latest reading from the appropriate sensors.
 *
 * Any value for a sensor that is disabled should be -1.
 */
int wearable_biometrics_read(wearable_sensor_reading_t *reading);


/*
 * Returns whether or not `sensor` is enabled and recording data.
 */
wearable_sensor_en_t wearable_biometric_sensor_get_enable(wearable_sensor_t sensor);

/*
 * Enable or disable `sensor` from recording values.
 */
void wearable_biometric_sensor_set_enable(wearable_sensor_t sensor, wearable_sensor_en_t);


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

