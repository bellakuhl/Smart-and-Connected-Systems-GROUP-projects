/* Isabella Kuhl, Laura Reeve, Joseph Rossi
* Main module for the security hub
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi.h"
#include "server_comm.h"
#include "driver/uart.h"
#include "driver/rmt.h"
#include "esp_err.h"
#include "esp_log.h"

#define RX_BUF_SIZE 1024
#define RMT_TX_CHANNEL    1     /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM  25     /*!< GPIO number for transmitter signal */
#define RMT_RX_CHANNEL    0     /*!< RMT channel for receiver */
#define RMT_RX_GPIO_NUM  34     /*!< GPIO number for receiver */

#define RED_GPIO        15
#define BLUE_GPIO       14
#define GREEN_GPIO      32
#define GPIO_button     4

#define RMT_CLK_DIV      100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

const int uart_num = UART_NUM_1;

static void transmit(char msg){

    uart_write_bytes(uart_num, &msg, 1);
    printf("Message sent: %c\n", msg);
}


static void tx_init(void){
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = 26;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 1;
    rmt_tx.tx_config.idle_level = 1;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = RMT_MODE_TX;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
    rmt_tx_start(RMT_TX_CHANNEL, false);
}


void receive(){
    // Receive data -- first two bytes are the code and latter two are the ID
    uint8_t data[32];
    while (1)
    {
        int reading = uart_read_bytes(uart_num, data, sizeof(data), 20/portTICK_PERIOD_MS);
        int start = -1;
        for (int i = 0; i<32; i++ ){
        	if (data[i]==0x80){
        		start = i;
        		break;
        	}
        }
        if ((start != -1) && (start+4 < 32)){
        	int i = start;
        	printf("DATA: %x %x %x %x %x\n", data[i], data[i+1], data[i+2], data[i+3], data[i+4]);
            uint16_t id = (uint16_t)data[i+1] << 8 | data[i+2];
            uint16_t code = (uint16_t)data[i+3] << 8 | data[i+4];
            printf("ID: %d, CODE: %d\n", id, code);

            int status = server_comm_make_request(id, code);
    		printf("Status: %d\n", status);

    		if (status == 200){
    			transmit('9');
    		}
    		else{
    			transmit('3');
    		}
        }
        memset(data,0,reading);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}


void app_main()
{
    wifi_init();
    wifi_connect();
    wifi_wait_for_ip();
    char *ip = wifi_get_ip_addr();

    uart_config_t config = {
        .baud_rate = 2400,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        //.rx_flow_ctrl_thresh = 0
    };

    uart_param_config(uart_num, &config);
    uart_set_line_inverse(uart_num, UART_INVERSE_RXD);
    uart_set_pin(uart_num, 25, 34, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, RX_BUF_SIZE, 0, 0, NULL, 0);

    tx_init();

    xTaskCreate(receive,"receive", 4096, NULL, 5, NULL);

//    int status = server_comm_make_request(1234, 6789);
//    printf("Status: %d\n", status);
}

