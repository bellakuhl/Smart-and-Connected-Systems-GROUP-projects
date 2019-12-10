/* Isabella Kuhl, Joseph Rossi, Laura Reeves
*  12/2/19
* This program implements a timer to record splits when beacon is detected
*/
#include "beacon_detection.h"
#include "server_communication.h"

#include "driver/timer.h"
#include "driver/rmt.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "string.h"
#include "esp_system.h"
#include <stdio.h>

#define DIVIDER 16
#define TIMER_SCALE (TIMER_BASE_CLK/DIVIDER)
// #define SPLITS_TIMER (0.00002*TIMER_SCALE) // 0.1 seconds
#define HW_TIMER_GROUP TIMER_GROUP_0
#define HW_TIMER_IDX 0
#define RXD_PIN (GPIO_NUM_25)
#define TXD_PIN (GPIO_NUM_12)

static const int RX_BUF_SIZE = 129;

static QueueHandle_t msgQueue = NULL;

static void uart_config(void)
{
   /* Configure parameters of an UART driver,
    * communication pins and install the driver */
    uart_config_t uart_config =
    {
            .baud_rate = 1200,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_line_inverse(UART_NUM_1, UART_INVERSE_RXD);
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

// Set up periodic timer for dt = 100ms
void beacon_rx_init(QueueHandle_t queue)
{
    uart_config();
    // Basic parameters of the timer
    timer_config_t config = {
        .alarm_en = 0,
        .counter_en = 0,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = true,
        .divider = DIVIDER
    };


    msgQueue = queue;

    // register timer interrupt
    timer_init(HW_TIMER_GROUP, HW_TIMER_IDX, &config);
    // Timer's counter will initially start from value below
    timer_set_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, 0);
    // Start timer
    timer_start(HW_TIMER_GROUP, HW_TIMER_IDX);
}

float beacon_rx_get_time()
{
    double time = 0;
    timer_get_counter_time_sec(HW_TIMER_GROUP, HW_TIMER_IDX, &time);
    return (float)time;
}


void beacon_rx_reset_timer()
{
    timer_set_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, 0);
}

void beacon_rx_task(void *arg)
{
   static const char *RX_TASK_TAG = "RX_TASK";
   esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
   uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
   memset(data, 0, RX_BUF_SIZE+1);


   while (1)
   {
        const int rxBytes = uart_read_bytes(
            UART_NUM_1, data, RX_BUF_SIZE, 100 / portTICK_RATE_MS);

        if (rxBytes > 0) {
            for (int i = 0; i < rxBytes; i++) {
                if (data[i] == 0x1B && msgQueue != NULL && i+3 < rxBytes) {
                    char cs = data[i]^data[i+1]^data[i+2];

                    if (data[i+1] != 'R' && data[i+1] != 'G' && data[i+1] != 'Y') {
                    //if (cs != data[i+3]) {
                        crawler_log("Unknown signal: %d", data[i+1]);
                    }
                    else {
                        BeaconMsg_t msg = { .color=data[i+1], .id=data[i+2] };
                        crawler_log("Received Msg: %d - %c", msg.id, msg.color);
                        BaseType_t res = xQueueSendToBack(msgQueue, &msg, 5);
                        if (res != pdTRUE) {
                            crawler_log("Error queing beacon message: %d", res);
                        }
                        uart_flush(UART_NUM_1);
                    }
                    break;
                }
            }
        }
        memset(data, 0, RX_BUF_SIZE+1);
   }
}
