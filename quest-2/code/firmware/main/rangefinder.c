#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "math.h"
#include "sensor_central.h"

#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   10          // Number of samples taken

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_5;  // GPIO33 ----> pin 33
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

int rangefinder_init()
{

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    return 0;
}

float rangefinder_read_meters()
{
    uint32_t adc_reading = 0;  // initialize adc_reading

    for (int i = 0; i < NO_OF_SAMPLES; i++) {  // take multiple samples
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
            adc_reading += raw;
        }
    }
    adc_reading /= NO_OF_SAMPLES;   // average samples

    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);  //Convert adc_reading to mV

    float puntualDistance = 60.374 * pow(voltage/1000.0, -1.16);  //  Formula from Sharp IR arduino library
    puntualDistance /= 100;   // convert to meters

    return puntualDistance;
}
