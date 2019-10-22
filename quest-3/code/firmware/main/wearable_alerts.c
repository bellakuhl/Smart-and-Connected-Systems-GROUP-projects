#include "wearable.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define GPIO_1 33 // Red LED
#define GPIO_2 15 // Blue LED

uint32_t seconds = 0;


static void water_alert(){
    while(1){
	    // Flash for five seconds
	    gpio_set_level(GPIO_2, 1); // Flash on
	    vTaskDelay(5000 / portTICK_PERIOD_MS);
	    gpio_set_level(GPIO_2, 0); // Flash off
	    vTaskDelay(seconds*1000 / portTICK_PERIOD_MS); // Wait 60s
	}
}

void wearable_schedule_alert(uint32_t period_sec)
{
	// Schedule an alert to flash every x seconds
	gpio_pad_select_gpio(GPIO_2);
    gpio_set_direction(GPIO_2, GPIO_MODE_OUTPUT);
    seconds = period_sec;
    xTaskCreate(water_alert,"water_alert", 4096, NULL, 5, NULL);
}

void wearable_trigger_alert()
{
	// Turns on red LED until user turns it off
	gpio_pad_select_gpio(GPIO_1);
    gpio_set_direction(GPIO_1, GPIO_MODE_OUTPUT);

    while (1){ // flash until turned off
    	gpio_set_level(GPIO_1, 1);
    }

}

