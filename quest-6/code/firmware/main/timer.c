/* Isabella Kuhl, Joseph Rossi, Laura Reeves
*  12/2/19
* This program implements a timer to record splits when beacon is detected
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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
#define RMT_TX_GPIO 26
#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RXD_PIN (GPIO_NUM_25)
#define TXD_PIN (GPIO_NUM_12)
static const int RX_BUF_SIZE = 1024;

// Flag for dt
static int dt_complete = 0;
static uint64_t split = 0;



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
   uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

// Set up periodic timer for dt = 100ms
void ir_rx_init()
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

    // register timer interrupt
    timer_init(HW_TIMER_GROUP, HW_TIMER_IDX, &config);
    // Timer's counter will initially start from value below
    timer_set_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, 0);
    // Start timer
    timer_start(HW_TIMER_GROUP, HW_TIMER_IDX);
}

void ir_rx_task(void *arg)
{
   char prevID = 0;
   static const char *RX_TASK_TAG = "RX_TASK";
   esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
   uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
   memset(data, 0, RX_BUF_SIZE+1);
   while (1) {
       const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 200 / portTICK_RATE_MS);
       uart_set_line_inverse(UART_NUM_1, UART_INVERSE_RXD);
       if (rxBytes > 0) {
        //  printf("Received Bytes: %d\n", rxBytes);
        //  ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
         for (int i = 0; i < rxBytes; i++) {
           if (data[i] == 0x1B) {
             timer_get_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, &split);
            //  printf("Split: %llu\n", split);
            //  printf("PrevID: %d\n", prevID);
            //  printf("Bytes: %d %d %d %d\n", data[i], data[i+1], data[i+2], data[i+3]);
             if (data[i+2] != prevID) {
               //ESP_LOGI(RX_TASK_TAG, "Read %d bytes: %c", rxBytes, data[i+1]);
               //ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
               timer_get_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, &split);
               printf("split: %.3f s\n", (float) split / TIMER_SCALE);
               timer_set_counter_value(HW_TIMER_GROUP, HW_TIMER_IDX, 0);
               printf("light: %c \n", data[i+1]);
               prevID = data[i+2];
             }
             break;
           }
         }
       }
       memset(data, 0, RX_BUF_SIZE+1);
   }
}

// void app_main()
// {
//     periodic_timer_init();
//     uart_config();
//     xTaskCreate(rx_task, "rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
// }
