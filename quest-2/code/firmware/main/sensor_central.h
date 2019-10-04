#include "pins.h"
#include "driver/adc.h"

#define ADC1_DEFAULT_VREF    1100
#define ADC2_DEFAULT_VREF    1100


int thermistor_mf2_init();
float thermistor_mf2_read_celcius();

int ultrasonic_init();
float ultrasonic_read_meters();

int rangefinder_init();
float rangefinder_read_meters();

int battery_monitor_int();
float battery_monitor_read_volts();
