#ifndef __T15_BEACON_DETECTION__
#define __T15_BEACON_DETECTION__

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct {
    char color;
    char id;
} BeaconMsg_t;

void ir_rx_init(QueueHandle_t queue);
float ir_rx_get_split_time();
void ir_rx_task(void *arg);

#endif