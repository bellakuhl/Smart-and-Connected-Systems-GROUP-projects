#include "wearable.h"

#define MOCK

#ifdef MOCK
// Mock implementation for development without a working circuit.
#include <time.h>
#include <stdlib.h>
static int steps = 0;
void wearable_sensors_init()
{
    srand(time(NULL));
}

int wearable_sensors_read(WearableSettings_t *settings, WearableSensorReading_t *reading)
{
    if (settings->battery_sensor_enabled) {
        reading->battery_volts = 2.5 + ((float)rand()/RAND_MAX * 5.5);
    }
    else {
        reading->battery_volts = -1;
    }

    if (settings->temperature_sensor_enabled) {
        reading->temperature_degc = 15.0f + ((float)rand()/RAND_MAX * 7);
    }
    else {
        reading->temperature_degc = -100.0f;
    }

    if (settings->step_sensor_enabled) {
        steps += ((float)rand()/RAND_MAX * 100);
        reading->steps = steps;
    }
    else {
        reading->steps = -1;
    }

    return 0;
}
#else
// Real Implementation Goes here
void wearable_sensors_init()
{
}

int wearable_sensors_read(WearableSettings_t *settings, WearableSensorReading_t *reading)
{
    return 0;
}
#endif

