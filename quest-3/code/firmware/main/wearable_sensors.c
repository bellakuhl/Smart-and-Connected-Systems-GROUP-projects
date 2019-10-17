#include "wearable.h"

#define MOCK

#ifdef MOCK
#include <time.h>
#include <stdlib.h>
void wearable_sensors_init()
{
    srand(time(NULL));
}

static int steps = 0;
int wearable_sensors_read(wearable_sensor_reading_t *reading)
{
    reading->battery_level_volts = 2.5 + ((float)rand()/RAND_MAX * 5.5);
    reading->body_temperature_degc = 15.0f + ((float)rand()/RAND_MAX * 7);
    steps += ((float)rand()/RAND_MAX * 100);
    reading->steps = steps;
    return 0;
}
#else

void wearable_sensors_init()
{
}

int wearable_sensors_read(wearable_sensor_reading_t *reading)
{
    return 0;
}
#endif

wearable_sensor_en_t wearable_sensor_get_enable(wearable_sensor_t sensor)
{
    return SENSOR_ENABLED;
}

void wearable_sensor_set_enable(wearable_sensor_t sensor, wearable_sensor_en_t en_state)
{
}

