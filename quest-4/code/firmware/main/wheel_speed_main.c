#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/pcnt.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_attr.h"
#include "esp_adc_cal.h"
#include "pulse_counter.h"
#include "alphanumeric_display.h"
#include "crawler.h"

//#define USE_GPIO
#define DEFAULT_VREF        1100

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6; //A2
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

#define DIAMETER_M  0.1778
void gpio_init()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, atten);

    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}

void read_gpio()
{
    uint32_t raw = adc1_get_raw((adc1_channel_t)channel);
    float volts = esp_adc_cal_raw_to_voltage(raw, adc_chars);

    alphadisplay_write_float(volts);
    printf("Raw: %u, Voltage: %.3f\n", raw, volts);
}

void gpio_task()
{
    while (1)
    {
        read_gpio();
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void update_display(void *arg)
{
    int16_t last_pulse_count = 0;
    float speed = 0;
    float period = 1000;
    while(1)
    {
        int16_t pulse_count = pulsecounter_get_count();
        float revolutions = (float)(pulse_count - last_pulse_count)/6.0f;
        float dist = 3.14159 * DIAMETER_M * revolutions;
        speed = dist/(period/1000.0f);
        alphadisplay_write_float(speed);
        printf("Count: %d, Last Count: %d, Speed: %.2f\n",
                pulse_count, last_pulse_count, speed);

        last_pulse_count = pulse_count;
        vTaskDelay(period/portTICK_PERIOD_MS);
    }
}


void app_main()
{
    alphadisplay_init();
    crawler_control_init();
    crawler_calibrate();
#ifdef USE_GPIO
    gpio_init();
    crawler_esc_set_value(PWM_NEUTRAL_US-100);
    xTaskCreate(gpio_task, "gpio_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);
#else
    pulsecounter_init();
    pulsecounter_start();
    xTaskCreate(update_display, "update_display", 4096, NULL, configMAX_PRIORITIES-1, NULL);
    //crawler_esc_set_value(PWM_NEUTRAL_US);
    crawler_esc_set_value(PWM_NEUTRAL_US-100);
#endif
}

