
/* Isabella Kuhl, Joseph Rossi, Laura Reeves
*  11/20/19
*  This program sends a user ID and passcode upon a button press.
*  It also continuously receives messages and if an unlock signal is received,
*  a green LED lights up, o/w a red one lights up.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include "string.h"
#include "esp_system.h"


/**
* - Port: UART1
* - Receive (Rx) buffer: on
* - Transmit (Tx) buffer: off
* - Flow control: off
* - Event queue: off
* - Pin assignment: see defines below
*/

#define RMT_TX_GPIO 26
#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define TXD_PIN (GPIO_NUM_25)
#define RXD_PIN (GPIO_NUM_34)

#define BUTTON_GPIO     4   // GPIO4 (A5) tactile button
#define RED 15
#define GREEN 32
#define LOCKED_SIGNAL '3'
#define UNLOCKED_SIGNAL '9'

static unsigned char code[] = {0x1f, 0x1d, 0x07, 0xce};
static const int RX_BUF_SIZE = 1024;
static int flag = 0;                  // interrupt flag
char currentMessage = LOCKED_SIGNAL;  // message number


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
     flag = 1;
}

static void rmt_tx_int(void)
{
   rmt_config_t config;
   config.rmt_mode = RMT_MODE_TX;
   config.channel = RMT_TX_CHANNEL;
   config.gpio_num = RMT_TX_GPIO;
   config.mem_block_num = 1;
   config.tx_config.loop_en = 0;
   // enable the carrier to be able to hear the Morse sound
   // if the RMT_TX_GPIO is connected to a speaker
   config.tx_config.carrier_en = 1;
   config.tx_config.idle_output_en = 1;
   config.tx_config.idle_level = 1;
   config.tx_config.carrier_duty_percent = 50;
   config.tx_config.carrier_freq_hz = 38000;
   config.tx_config.carrier_level = 1;
   // set the maximum clock divider to be able to output
   // RMT pulses in range of about one hundred milliseconds
   config.clk_div = 255;

   ESP_ERROR_CHECK(rmt_config(&config));
   ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
   rmt_tx_start(config.channel, 1);
}

static void uart_config(void)
{
   /* Configure parameters of an UART driver,
    * communication pins and install the driver */
    uart_config_t uart_config =
    {
            .baud_rate = 2400,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
   uart_param_config(UART_NUM_1, &uart_config);
   uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
   uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

// change the LED based on LOCK/UNLOCK state
static void switch_LED_task()
{
     if (currentMessage == LOCKED_SIGNAL) // while fob is locked, light red LED
     {
       gpio_set_level(RED, 1);
       gpio_set_level(GREEN, 0);
     }
     else if (currentMessage == UNLOCKED_SIGNAL)  // when unlocked, light green LED
     {
       gpio_set_level(RED, 0);
       gpio_set_level(GREEN, 1);
     }
}

// send data to uart
int sendData(const char* logName, const char* data)
{
   const int txBytes = uart_write_bytes(UART_NUM_1, code, sizeof(code));
   ESP_LOGI(logName, "Wrote %s, %d bytes", data, txBytes);
   return txBytes;
}

static void tx_task(void *arg)
{
   static const char *TX_TASK_TAG = "TX_TASK";
   esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
   while (1) {
     vTaskDelay(100 / portTICK_RATE_MS);
     if (flag == 1)
     {
       sendData(TX_TASK_TAG, currentMessage);
     }
     flag = 0;
   }
}

// receive state
static void rx_task(void *arg)
{
   static const char *RX_TASK_TAG = "RX_TASK";
   esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
   uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
   memset(data, 0, RX_BUF_SIZE+1);  // clear data buffer
   while (1) {
       const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 100 / portTICK_RATE_MS);
       uart_set_line_inverse(UART_NUM_1, UART_INVERSE_RXD); // inverse message
       if (rxBytes > 0) {
         currentMessage = data[0];  // set currentMessage
         if (currentMessage == LOCKED_SIGNAL)
         {
           switch_LED_task();
         }
         else if (currentMessage == UNLOCKED_SIGNAL)
         {
           switch_LED_task();
           vTaskDelay(5000 / portTICK_RATE_MS);
           currentMessage = LOCKED_SIGNAL;
           switch_LED_task();
         }
         ESP_LOGI(RX_TASK_TAG, "Read %d bytes: %c", rxBytes, data[0]);
         ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
       }
   }
   free(data);
}

void app_main(void)
{
   rmt_tx_int();  // initialize RMT to send 38kHz signal
   uart_config(); // initialize UART

   gpio_pad_select_gpio(RED);
   gpio_pad_select_gpio(GREEN);
   gpio_pad_select_gpio(BUTTON_GPIO);
   gpio_set_direction(RED, GPIO_MODE_OUTPUT);
   gpio_set_direction(GREEN, GPIO_MODE_OUTPUT);
   gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);

   gpio_set_level(RED, 1);  // initialize red LED to be ON
   gpio_set_level(GREEN, 0);

   gpio_intr_enable(BUTTON_GPIO);     // enable interrupt
   gpio_set_intr_type(BUTTON_GPIO, GPIO_PIN_INTR_POSEDGE); // trigger interrupt on positive edge

   gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
   gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, (void*) BUTTON_GPIO);

   xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-2, NULL);
   xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}
