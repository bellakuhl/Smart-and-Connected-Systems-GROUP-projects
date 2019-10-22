// Joseph Rossi, Isabella Kuhl, Laura Reeve
#ifndef __TEAM15_WEARABLE__
#define __TEAM15_WEARABLE__

#include <stdint.h>

typedef enum {
    WEARABLE_SENSOR_STEP = 0,
    WEARABLE_SENSOR_TEMPERATURE,
    WEARABLE_SENSOR_BATTERY,
    WEARABLE_NUM_SENSORS
} WearableSensor_t;


typedef enum {
    SENSOR_DISABLED = 0,
    SENSOR_ENABLED
} WearableSensorEn_t;


typedef struct {
    /* The current body temperature in degc. */
    float temperature_degc;

    /* The current batter level in volts. */
    float battery_volts;

    /* The number of steps recorded. */
    int32_t steps;
} WearableSensorReading_t;


typedef struct {
    int8_t battery_sensor_enabled;
    int8_t temperature_sensor_enabled;
    int8_t step_sensor_enabled;
    uint8_t alert_now;
    uint32_t alert_period_sec;
} WearableSettings_t;


/**
 * When called this funciton should initialize all sensors (configure GPIOs, PWMs, etc)
 * so they are ready to be read via `wearable_sensors_read`.
 */
void wearable_sensors_init();


/*
 * When called, the `reading` argument should be populated with
 * the latest reading from the appropriate sensors.
 *
 * If the sensors are disabled, the following values are expected:
 *   temperature_degc: -100
 *   battery_volts: -1
 *   steps: -1
 */
int wearable_sensors_read(WearableSettings_t *settings, WearableSensorReading_t *reading);


/*
 * Schedules alerts to trigger repeatedly every `period_sec` seconds.
 * To disable alerts,  pass `period_sec` as 0.
 */
void wearable_schedule_alert(uint32_t period_sec);


/*
 * Used to trigger an alert immediately.
 */
void wearable_trigger_alert();

void accel_init();
int accel_step_count();

int thermistor_mf2_init();
float thermistor_mf2_read_celcius();

int battery_monitor_int();
float battery_monitor_read_volts();



#endif

