/*
*  Isabella Kuhl, Joseph Rossi, Laura Reeve
*  This program sets up alerts for water and location
*
*/

#include "wearable.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define GPIO_1 GPIO_NUM_33 // Red LED
#define GPIO_2 GPIO_NUM_15 // Blue LED

uint32_t seconds = 0;

static void water_alert_task(){
    while(1) {
        printf("Flash water\n");
	    // Flash for five seconds
	    gpio_set_level(GPIO_2, 1); // Flash on
	    vTaskDelay(seconds*1000 / portTICK_PERIOD_MS);

	    gpio_set_level(GPIO_2, 0); // Flash off
	    vTaskDelay(seconds*1000 / portTICK_PERIOD_MS); // Wait 60s
	}
}

static int water_alert_init = 0;
static TaskHandle_t water_alert_task_handle = NULL;
void wearable_schedule_water_alert(uint32_t period_sec)
{
	// Schedule an alert to flash every x seconds
    if (water_alert_init == 0) {
        gpio_pad_select_gpio(GPIO_2);
        gpio_set_direction(GPIO_2, GPIO_MODE_OUTPUT);
        water_alert_init = 1;
    }

    seconds = period_sec;
    if (water_alert_task_handle != NULL) {
        vTaskDelete(water_alert_task_handle);
        water_alert_task_handle = NULL;
    }

    xTaskCreate(water_alert_task,"water_alert_task", 4096, NULL, 5, &water_alert_task_handle);
}

void find_device_task() {
    while (1){ // flash until turned off
        printf("Flash find\n");
    	gpio_set_level(GPIO_1, 1);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    	gpio_set_level(GPIO_1, 0);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

static int find_device_init_called = 0;
static TaskHandle_t find_device_task_handle = NULL;
void wearable_find_device(bool enable)
{
    if (find_device_init_called == 0) {
        gpio_pad_select_gpio(GPIO_1);
        gpio_set_direction(GPIO_1, GPIO_MODE_OUTPUT);
        find_device_init_called = 1;
    }

	// Turns on red LED until user turns it off
    if (enable && find_device_task_handle == NULL) {
        xTaskCreate(find_device_task,"find_device_task", 4096, NULL, 5, &find_device_task_handle);
    }
    else if (find_device_task_handle != NULL) {
        vTaskDelete(find_device_task_handle);
    }
}
