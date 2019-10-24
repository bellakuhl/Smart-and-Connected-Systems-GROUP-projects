/*
*  Isabella Kuhl, Joseph Rossi, Laura Reeve
*  This program collects data from the sensors
*/

#include "wearable.h"

// Real Implementation Goes here
void wearable_sensors_init()
{
    thermistor_mf2_init();
    battery_monitor_int();
    accel_init();
}

int wearable_sensors_read(WearableSettings_t *settings, WearableSensorReading_t *reading)
{
    if (settings->battery_sensor_enabled) {
        reading->battery_volts = battery_monitor_read_volts();
    }
    else {
        reading->battery_volts = -1;
    }

    if (settings->temperature_sensor_enabled) {
        reading->temperature_degc = thermistor_mf2_read_celcius();
    }
    else {
        reading->temperature_degc = -100.0f;
    }

    if (settings->step_sensor_enabled) {
        reading->steps = accel_step_count();
    }
    else {
        reading->steps = -1;
    }
    return 0;
}

