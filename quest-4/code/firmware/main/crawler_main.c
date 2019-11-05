#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "driver/uart.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "alphanumeric_display.h"
#include "crawler_control.h"
#include "pid_control.h"
#include "pulse_counter.h"
#include "ultrasonic.h"
#include "lidar.h"
#include "wifi.h"

#define DIAMETER_M 0.1778
#define CMD_RECV_PORT 8080

enum CrawlerCommands {
    CMD_ESC = 0,
    CMD_STEER,
    CMD_START_AUTO,
    CMD_STOP_AUTO
};

typedef enum {
    CRAWL_STATE_AUTO,
    CRAWL_STATE_STOPPED
} CrawlerState_t;

static CrawlerState_t crawler_state = CRAWL_STATE_STOPPED;

#pragma pack(push, 1)
typedef struct {
    // This should be one of the CrawlerCommand values, but
    // since it's a message received via UDP, I want to
    // specifcy a predictable size.
    uint8_t cmd;
    uint16_t value;
} CrawlerCmd_t;
#pragma pack(pop)

static void crawler_send_msg(const char *msg)
{
    char addr_str[128];
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = inet_addr("192.168.1.109");
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(8080);
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(char)*128 - 1);

    int err = sendto(
        sock,
        msg,
        strlen(msg),
        0,
        (struct sockaddr *)&destAddr,
        sizeof(destAddr)
    );

    if (err < 0) {
        // Dont call log again so we don't end up
        // in an infinite cyclical call.
        printf("Error sending message: %d\n", err);
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
    me.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(sock, (struct sockaddr*) &me, sizeof(me))==-1)
        printf("Bind failed\n");

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
        printf("Received\n");

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
            printf("Len: %d\n", len);

            CrawlerCmd_t *data = (CrawlerCmd_t *)rx_buffer;
            crawler_log("Received Message - Cmd: %d, value %d\n", data->cmd, data->value);
            switch(data->cmd) {
                case CMD_ESC:
                    mcpwm_set_duty_in_us(ESC_PWM_UNIT, ESC_PWM_TIMER, MCPWM_OPR_A, data->value);
                    break;
                case CMD_STEER:
                    mcpwm_set_duty_in_us(STEERING_PWM_UNIT, STEERING_PWM_TIMER, MCPWM_OPR_A, data->value);
                    break;
                case CMD_START_AUTO:
                    crawler_esc_set_value(PWM_NEUTRAL_US - 100);
                    crawler_state = CRAWL_STATE_AUTO;
                    break;
                case CMD_STOP_AUTO:
                    crawler_state = CRAWL_STATE_STOPPED;
                    crawler_esc_set_value(PWM_NEUTRAL_US);
                    break;
                default:
                    crawler_log("Unknown command type: %d", data->cmd);
            }
        }
    }
}

void distance_sensor_task()
{
    ultrasonic_serial_init();

    while (1)
    {
        float dist = ultrasonic_read_latest();
        crawler_log("Distance: %f\n", dist);
        if (dist <= 0.32f && crawler_state == CRAWL_STATE_AUTO) {
            crawler_esc_set_value(PWM_NEUTRAL_US);
            crawler_state = CRAWL_STATE_STOPPED;
        }
    }
}

void crawler_speed_monitor()
{
    int16_t last_pulse_count = 0;
    float speed = 0;
    float period = 1000;

    while(1)
    {
        if (crawler_state == CRAWL_STATE_AUTO)
        {
            int16_t pulse_count = pulsecounter_get_count();
            float revolutions = (float)(pulse_count - last_pulse_count)/6.0f;
            float dist = 3.14159 * DIAMETER_M * revolutions;

            speed = dist/(period/1000.0f);
            speed *= crawler_get_direction();

            crawler_log("Speed: %.2f, PC: %u, LPC: %u, Delta: %u\n",
                    speed, pulse_count, last_pulse_count, pulse_count - last_pulse_count);
            alphadisplay_write_float(speed);
            last_pulse_count = pulse_count;

            float adjustment = PID(speed);
            float pwm_adjust = adjustment;

            crawler_log("Adjustment: %f\n", adjustment);
            crawler_esc_set_value(crawler_esc_get_value() - pwm_adjust);
        }
        vTaskDelay(period/portTICK_PERIOD_MS);
    }
}
#define LIDAR_FRONT_UART UART_NUM_0
#define LIDAR_REAR_UART UART_NUM_2
#define LIDAR_FRONT GPIO_NUM_33
#define LIDAR_REAR  GPIO_NUM_14

void lidar_monitor()
{
    while (1)
    {
        uint32_t front_dist = 0;
        uint32_t front_stren = 0;
        uint32_t rear_dist;
        uint32_t rear_stren;

        lidar_read(LIDAR_FRONT_UART, &front_dist, &front_stren);
        lidar_read(LIDAR_REAR_UART, &rear_dist, &rear_stren);

        crawler_log("Front: %d, Rear: %d\n", front_dist, rear_dist);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}


void app_main()
{
    alphadisplay_init();
    pulsecounter_init();
    pulsecounter_start();
    lidar_init(LIDAR_FRONT_UART, LIDAR_FRONT);
    lidar_init(LIDAR_REAR_UART, LIDAR_REAR);

    wifi_init();
    wifi_connect((uint8_t *)"Group_15", 9, (uint8_t *)"smart-systems", 14);
    wifi_wait_for_ip();
    crawler_log("Connected\n");

    crawler_control_init();
    //crawler_calibrate();
    crawler_steering_set_value(PWM_NEUTRAL_US);
    vTaskDelay(2000/portTICK_PERIOD_MS);

    PID_set_setpoint(0.1);
    PID_init();

    //xTaskCreate(crawler_cmd_recv, "crawler_cmd_recv", 4096, NULL, configMAX_PRIORITIES-2, NULL);
    //xTaskCreate(crawler_speed_monitor, "crawler_speed_monitor", 4096, NULL, configMAX_PRIORITIES-1, NULL);
    //xTaskCreate(distance_sensor_task, "distance_sensor_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(lidar_monitor, "lidar_monitor", 4096, NULL, configMAX_PRIORITIES-1, NULL);
}

