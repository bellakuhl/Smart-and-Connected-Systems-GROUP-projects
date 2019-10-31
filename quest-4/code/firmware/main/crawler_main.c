#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "wifi.h"

#define STEERING_PWM_UNIT   MCPWM_UNIT_0
#define STEERING_PWM_TIMER  MCPWM_TIMER_0
#define STEERING_PWM_PIN    MCPWM0A
#define STEERING_PWM_GPIO   GPIO_NUM_15

#define ESC_PWM_UNIT   MCPWM_UNIT_0
#define ESC_PWM_TIMER  MCPWM_TIMER_1
#define ESC_PWM_PIN    MCPWM1A
#define ESC_PWM_GPIO   GPIO_NUM_12

#define PWM_LOW_US 900
#define PWM_HIGH_US 2400
#define PWM_NEUTRAL_US 1500

#define CMD_RECV_PORT 8080

const uint8_t CMD_ESC = 0;
const uint8_t CMD_STEER = 1;

#pragma pack(push, 1)
typedef struct {
    uint8_t cmd;
    uint16_t value;
} CrawlerCmd_t;
#pragma pack(pop)

static void crawler_send_msg(const char *msg)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = INADDR_BROADCAST;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(8080);

    int err = sendto(
        sock,
        msg,
        strlen(msg),
        0,
        (struct sockaddr *)&destAddr,
        sizeof(destAddr)
    );

    if (err) {
        // Dont call log again so we don't end up
        // in an infinite cyclical call.
        printf("Error sending message.");
    }

}

static void crawler_log(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    printf(buffer);
    crawler_send_msg(buffer);
}

#define RX_BUFFER_SIZE 128
static void crawler_cmd_recv()
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    struct sockaddr_in me;
    memset((char *) &me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_port = htons(CMD_RECV_PORT);
    me.sin_addr.s_addr = inet_addr(wifi_get_ip_addr());

    uint8_t rx_buffer[RX_BUFFER_SIZE];

    while (1)
    {
        struct sockaddr_in fromAddr;
        socklen_t socklen = sizeof(fromAddr);
        crawler_log("Waiting to receive...\n");
        int len = recvfrom(
            sock,
            rx_buffer,
            sizeof(uint8_t)*RX_BUFFER_SIZE - 1,
            0,
            (struct sockaddr *)&fromAddr,
            &socklen
        );

        if (len < 0) {
            crawler_log("Failed to recv from socket: %d", errno);
        }
        else {
            // Perhaps we shoudl check the first few bytes to parse
            // what kind of message this is, but this is easier for now.
            if (len != sizeof(CrawlerCmd_t)) {
                crawler_log("Ignoring unknown message size: %d\n", len);
                continue;
            }

            CrawlerCmd_t *data = (CrawlerCmd_t *)rx_buffer;
            crawler_log("Received Message - Cmd: %d, value %d\n", data->cmd, data->value);
            if (data->cmd == CMD_ESC) {
                mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, data->value);
            }
            else {
                mcpwm_set_duty_in_us(STEERING_PWM_UNIT, STEERING_PWM_TIMER, MCPWM_OPR_A, data->value);
            }
        }
    }
}

void crawler_control_init()
{
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;
    pwm_config.cmpr_a = 0; pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(STEERING_PWM_UNIT, STEERING_PWM_TIMER, &pwm_config);
    mcpwm_init(ESC_PWM_UNIT, ESC_PWM_TIMER, &pwm_config);

    mcpwm_gpio_init(STEERING_PWM_UNIT, STEERING_PWM_PIN, STEERING_PWM_GPIO);
    mcpwm_gpio_init(ESC_PWM_UNIT, ESC_PWM_PIN, ESC_PWM_GPIO);

    vTaskDelay(500/portTICK_PERIOD_MS);
}

void crawler_calibrate()
{
    crawler_log("Calibrating...\n");
    vTaskDelay(3000 / portTICK_PERIOD_MS);  // Give yourself time to turn on crawler
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, PWM_HIGH_US); // HIGH signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, PWM_LOW_US);  // LOW signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, PWM_NEUTRAL_US); // NEUTRAL signal in microseconds
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, PWM_NEUTRAL_US); // reset the ESC to neutral (non-moving) value
    crawler_log("Calibration finished.");
}


void app_main()
{
    wifi_init();
    wifi_connect((uint8_t *)"Group_15", 9, (uint8_t *)"smart-systems", 14);
    wifi_wait_for_ip();
    crawler_log("Connected");

    crawler_control_init();
    crawler_calibrate();

    xTaskCreate(crawler_cmd_recv, "crawler_cmd_recv", 4096, NULL, configMAX_PRIORITIES-1, NULL);
}

