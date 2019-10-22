/* This file is based on the example blink.c code for LED flashing */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"


#define GPIO_1 33 // Red LED
#define GPIO_2 15 // Blue LED


static void water_alert(){
	gpio_pad_select_gpio(GPIO_2);
    gpio_set_direction(GPIO_2, GPIO_MODE_OUTPUT);
    
    while(1){
	    // Flash for five seconds
	    gpio_set_level(GPIO_2, 1); // Flash on
	    vTaskDelay(5000 / portTICK_PERIOD_MS);
	    gpio_set_level(GPIO_2, 0); // Flash off
	    vTaskDelay(60000 / portTICK_PERIOD_MS); // Wait 60s
	}
}

static void find_device() {
	gpio_pad_select_gpio(GPIO_1);
    gpio_set_direction(GPIO_1, GPIO_MODE_OUTPUT);

    while (1){ // flash until turned off
    	gpio_set_level(GPIO_1, 1);
    	// check if turned off alert
    	// if it's turned off, break;
    }
}

void water_alert_init() {
	// Alerts every minute to drink water (for demonstrative purpose)
	xTaskCreate(water_alert,"water_alert", 4096, NULL, 5, NULL);
}