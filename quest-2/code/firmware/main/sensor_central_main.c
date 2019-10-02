
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#include "sensor_central.h"

//#define MOCK

typedef struct {
    float battery_volts;
    float ultrasonic_m;
    float rangefinder_m;
    float thermistor_degc;
} sensor_reading_t;


void serialize_reading_json(sensor_reading_t *reading, char **dst)
{
    // { 'battery': %
    char *fmt = "{\"battery_volts\":%.3f,\"ultrasonic_m\":%.3f,"
                "\"rangefinder_m\": %.3f,\"thermistor_degc\":%.3f}";

    size_t strsize = sizeof(char) * strlen(fmt) + 5 * sizeof(char) + 50; // 50 padding
    *dst = (char *)malloc(strsize);
    memset(*dst, 0, strsize);
    if (*dst == NULL) {
        return; // malloc failed, handle error?
    }

    sprintf(
        *dst, fmt,
        reading->battery_volts,
        reading->ultrasonic_m,
        reading->rangefinder_m,
        reading->thermistor_degc
    );
}

#ifndef MOCK
sensor_reading_t read_sensors()
{
    sensor_reading_t reading = {
        .battery_volts = battery_monitor_read_volts(),
        .ultrasonic_m = ultrasonic_read_meters(),
        .rangefinder_m = rangefinder_read_meters(),
        .thermistor_degc = thermistor_mf2_read_celcius()
    };
    return reading;
}
#else
#include <time.h>

sensor_reading_t read_sensors()
{
    sensor_reading_t reading = {
        .battery_volts = 2.5 + ((float)rand()/RAND_MAX * 5.5),
        .ultrasonic_m = 0.5 + ((float)rand()/RAND_MAX * 15),
        .rangefinder_m = 0.5 + ((float)rand()/RAND_MAX * 15),
        .thermistor_degc = 15.0f + ((float)rand()/RAND_MAX * 7)
    };

    return reading;
}
#endif


void app_main(void)
{
#ifdef MOCK
    srand(time(NULL));
#endif

    thermistor_mf2_init();
    ultrasonic_init();
    rangefinder_init();
    battery_monitor_int();

    while (1)
    {

        char *json = NULL;
        sensor_reading_t reading = read_sensors();
        serialize_reading_json(&reading, &json);

        if (json != NULL) {
            printf("%s\n", json);
            fflush(stdout);
            free(json);
        }

        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

