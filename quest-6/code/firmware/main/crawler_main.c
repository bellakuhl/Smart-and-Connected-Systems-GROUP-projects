/* Isabella Kuhl, Laura Reeves, Joesph Roosi
* Program for controlling sensors and sending to server
*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "driver/uart.h"
#include "server_communication.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "alphanumeric_display.h"
#include "crawler_control.h"
#include "pid_control.h"
#include "pulse_counter.h"
#include "ultrasonic.h"
#include "lidar.h"
#include "lidarlite.h"
#include "wifi.h"
#include "beacon_detection.h"

// This wheel diameter, used to calculate the wheel
// speed based on the pulse counter.
#define DIAMETER_M 0.1778

#define CMD_RECV_PORT CONFIG_CRAWLER_UDP_RECV_PORT
#define SERVER_HOST CONFIG_SERVER_HOST
#define SERVER_UDP_PORT CONFIG_SERVER_UDP_PORT

#define UDP_RX_BUFFER_SIZE 128

// When starting in AUTO mode, use this speed
// as the default.
#define CRAWLER_START_PWM (PWM_NEUTRAL_US - 150)

const static char *TAG = "Crawler Main";

enum CrawlerCommands {
    CMD_ESC = 0,
    CMD_STEER,
    CMD_START_AUTO,
    CMD_STOP_AUTO,
    CMD_CALIBRATE
};

typedef enum {
    CRAWL_STATE_AUTO,
    CRAWL_STATE_MANUAL,
    CRAWL_STATE_STOPPED
} CrawlerState_t;

typedef enum {
    CRAWL_AUTO_BEACON,
    CRAWL_AUTO_STRAIGHT,
    CRAWL_AUTO_LEFT_TURN
} CrawlerAutoState_t;

// TODO: Turn these into a struct if time
static CrawlerState_t crawler_state = CRAWL_STATE_STOPPED;
static CrawlerAutoState_t crawler_auto_state = CRAWL_AUTO_BEACON;
static float crawler_current_speed = 0;
static int current_pulse_count = 0;
static int last_pulse_count = 0;
static TaskHandle_t crawler_auto_mode_task = NULL;
static float collision_dist;
static uint32_t right_side_front_dist;
static uint32_t right_side_rear_dist;
static QueueHandle_t beaconMsgQueue;
static float total_revolutions = 0;


#pragma pack(push, 1)
typedef struct {
    // This should be one of the CrawlerCommand values, but
    // since it's a message received via UDP, I want to
    // specifcy a predictable size.
    uint8_t cmd;
    uint16_t value;
} CrawlerCmd_t;
#pragma pack(pop)


static void crawler_stop()
{
    // TODO: If time, make it stop gracefully
    crawler_esc_set_value(PWM_NEUTRAL_US);
}


static void distance_sensor_task()
{
    collision_dist = lidar_lite_get_distance();
}

static float speed_monitor_period = 2000;
static void crawler_speed_monitor()
{
    while (1)
    {
        current_pulse_count = pulsecounter_get_count();
        float revolutions = (float)(current_pulse_count - last_pulse_count)/6.0f;
        total_revolutions += revolutions;
        float dist = 3.14159 * DIAMETER_M * revolutions;
        crawler_current_speed = dist/(speed_monitor_period/1000.0f);
        crawler_current_speed *= crawler_get_direction();

        last_pulse_count = current_pulse_count;
        alphadisplay_write_float(crawler_current_speed);
        vTaskDelay(speed_monitor_period/portTICK_PERIOD_MS);
    }
}

#define LIDAR_FRONT_UART UART_NUM_0
#define LIDAR_BACK_UART UART_NUM_2
#define LIDAR_FRONT GPIO_NUM_33
#define LIDAR_BACK  GPIO_NUM_14

static void side_distance_monitor()
{
    uint32_t front_stren = 0;
    uint32_t back_stren;

    lidar_read(LIDAR_FRONT_UART, &right_side_front_dist, &front_stren);
    lidar_read(LIDAR_BACK_UART, &right_side_rear_dist, &back_stren);
}

static bool should_turn_left()
{
    // return collision_dist <= 50; // JR - Test

    // return collision_dist <= 310;

    // With more responsive sensor readings I want
    // to try this again.
    return collision_dist <= 260;

}

static int reading_count = 0;
static void update_distance_sensors()
{
    while (1)
    {
        distance_sensor_task();
        side_distance_monitor();

        reading_count = (reading_count + 1) % 20;
        if (reading_count == 0) {
            // crawler_log("Front: %.2d\tBack: %.2d\tSteering Val: %.2d",
            //             right_side_front_dist, right_side_rear_dist, crawler_steering_get_value());
            crawler_log("Lidar Lite: %.2f\n", collision_dist);
        }

        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

static void crawl_autonomous_task()
{
    PID_set_setpoint(0.7);

    crawler_auto_state = CRAWL_AUTO_BEACON;
    int beacon_ids[3] = {-1};
    int beacon_count = 0;
    int collision_trigger_count = 0;
    int loop_period_ms = 20;
    int pid_period_ms = speed_monitor_period / loop_period_ms;
    int last_pid_control = 0;

    BeaconMsg_t msg;

    TaskHandle_t beacon_task;
    xQueueReset(beaconMsgQueue);
    xTaskCreate(beacon_rx_task, "beacon_rx_task", 4096, NULL,
                configMAX_PRIORITIES-1, &beacon_task);

    float start_revolutions = 0;
    while (1)
    {
        if (crawler_state == CRAWL_STATE_AUTO && collision_dist <= 30.0f)
        {
            collision_trigger_count++;
            if (collision_trigger_count >= 2) {
                crawler_log("Stop Distance: %f\n", collision_dist);
                crawler_stop();
                crawler_state = CRAWL_STATE_STOPPED;
            }
        }
        else {
            collision_trigger_count = 0;
        }

        if (crawler_state != CRAWL_STATE_AUTO) {
            // crawler_log("Not in auto mode");
            vTaskDelay(100/portTICK_PERIOD_MS);
            if (beacon_task != NULL) {
                vTaskDelete(beacon_task);
                beacon_task = NULL;
            }
            continue;
        }

        if (crawler_auto_state == CRAWL_AUTO_BEACON)
        {
            if (beacon_count > 0) {
                // TODO: Log the split time to the server
                float split = beacon_count == 1 ? 0 : beacon_rx_get_time();
                server_log_split_time(msg.id, split, NULL, NULL, 0);
                // alphadisplay_write_float(split);
            }

            if (beacon_count >= 3) {
                crawler_log("Third beacon encountered, exiting auto mode.\n");
                crawler_state = CRAWL_STATE_STOPPED;
                crawler_stop();
            }
            else if (beacon_count > 0) {
                crawler_log("Waiting for a green lite");
                while (1) {
                    if (msg.color == 'R') {
                        // If the light is red, stop and keep draining
                        // the queue
                        crawler_stop();
                        xQueueReceive(beaconMsgQueue, &msg, 10);
                    }
                    else if (msg.color == 'G' || msg.color == 'Y') {
                        // Start counting from the first green light.
                        if (beacon_count == 1) {
                            beacon_rx_reset_timer();
                        }
                        crawler_log("Green Light - Start going straight");
                        crawler_auto_state = CRAWL_AUTO_STRAIGHT;
                        break;
                    }
                    else {
                        crawler_log("Unknown color: %c", msg.color);
                    }
                }
            }
        }
        else if (crawler_auto_state == CRAWL_AUTO_STRAIGHT)
        {
            if (last_pid_control == 0) {
                if (crawler_current_speed == 0)
                {
                    crawler_esc_set_value(CRAWLER_START_PWM);
                }
                else {
                    float adjustment = PID(crawler_current_speed);
                    float pwm_adjust = adjustment;
                    printf("Adjustment: %.3f\n", adjustment);
                    crawler_esc_set_value(crawler_esc_get_value() - pwm_adjust);
                    last_pid_control = 0;
                }
            }
            else {
                // We only want to use PID control at the same interface as the
                // speed monitor so things don't go crazy.
                last_pid_control = last_pid_control + 1 % pid_period_ms;
            }

            // // Keep us going straight
            // uint32_t diff = right_side_rear_dist - right_side_front_dist;
            // int value = diff*7 + PWM_NEUTRAL_US;
            // crawler_steering_set_value(value);
            crawler_steering_set_value(PWM_NEUTRAL_US);

            if (should_turn_left()) {
                start_revolutions = total_revolutions;
                crawler_log("Start Left Turn - Dist: %.2f, Star Rev: %.2f\n", collision_dist, start_revolutions);
                crawler_steering_set_value(PWM_HIGH_US);
                crawler_auto_state = CRAWL_AUTO_LEFT_TURN;
            }
            // Could implement "if should_turn_right here"
        }
        else if (crawler_auto_state == CRAWL_AUTO_LEFT_TURN)
        {
            // Turn all the way left, do not control speed during turn! (yet...)
            // we're intentionally not blocking in this state so collision
            // detection will keep us from hitting anything
            if (total_revolutions - start_revolutions >= 6.5) {
                crawler_log("Turn Finished: %.2f\n", total_revolutions);
                crawler_steering_set_value(PWM_NEUTRAL_US);
                crawler_auto_state = CRAWL_AUTO_STRAIGHT;
                // beacon_ids[0] = -1; //JR - test auto
                // beacon_ids[1] = -1; //JR - test auto
                // beacon_ids[2] = -1; //JR - test auto
            }
        }

        // Draining the beacon message queue looking for new beacon id
        while (xQueueReceive(beaconMsgQueue, &msg, 0) == pdTRUE)
        {
            crawler_log("Reading queue");
            bool seen = false;
            for (int i = 0; i < beacon_count; i++) {
                if (beacon_ids[i] == msg.id) {
                    seen = true;
                    break;
                }
            }

            // We have not seen this beacon
            if (!seen) {
                crawler_auto_state = CRAWL_AUTO_BEACON;
                beacon_ids[beacon_count++] = msg.id;
                crawler_log("New beacon found: %d (total - %d)\n", msg.id, beacon_count);
                break;
            }
        }

        vTaskDelay(loop_period_ms/portTICK_PERIOD_MS);
    }
}

static void crawler_cmd_recv()
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    struct sockaddr_in me;
    memset((char *) &me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_port = htons(CMD_RECV_PORT);
    me.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(sock, (struct sockaddr*) &me, sizeof(me))==-1)
        ESP_LOGE(TAG, "Bind failed\n");

    uint8_t rx_buffer[UDP_RX_BUFFER_SIZE];

    while (1)
    {
        struct sockaddr_in fromAddr;
        socklen_t socklen = sizeof(fromAddr);
        crawler_log("Waiting to receive...\n");
        int len = recvfrom(
            sock,
            rx_buffer,
            sizeof(uint8_t)*UDP_RX_BUFFER_SIZE - 1,
            0,
            (struct sockaddr *)&fromAddr,
            &socklen
        );
        ESP_LOGD(TAG, "Received\n");

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
            ESP_LOGD(TAG, "Len: %d\n", len);

            CrawlerCmd_t *data = (CrawlerCmd_t *)rx_buffer;
            crawler_log("Received Message - Cmd: %d, value %d\n",
                         data->cmd, data->value);
            switch(data->cmd) {
                case CMD_ESC:
                    if (crawler_state != CRAWL_STATE_AUTO) {
                        crawler_esc_set_value(data->value);
                        if (data->value != PWM_NEUTRAL_US) {
                            crawler_state = CRAWL_STATE_MANUAL;
                        }
                    }
                    break;
                case CMD_STEER:
                    if (crawler_state != CRAWL_STATE_AUTO) {
                        crawler_steering_set_value(data->value);
                    }
                    break;
                case CMD_START_AUTO:
                    crawler_state = CRAWL_STATE_AUTO;
                    if (crawler_auto_mode_task == NULL) {
                        alphadisplay_write_ascii(0, 'A');
                        alphadisplay_write_ascii(1, 'U');
                        alphadisplay_write_ascii(2, 'T');
                        alphadisplay_write_ascii(3, '0');
                        xTaskCreate(crawl_autonomous_task, "", 4096, NULL, configMAX_PRIORITIES-1, &crawler_auto_mode_task);
                    }
                    break;
                case CMD_STOP_AUTO:
                    // This should caue the auto task to return, but we need
                    // to delete the task anyway.
                    crawler_state = CRAWL_STATE_STOPPED;
                    crawler_stop();
                    if (crawler_auto_mode_task != NULL) {
                        vTaskDelete(crawler_auto_mode_task);
                        crawler_auto_mode_task = NULL;
                    }
                    break;
                case CMD_CALIBRATE:
                    crawler_calibrate();
                    break;
                default:
                    crawler_log("Unknown command type: %d", data->cmd);
            }
        }
    }
}


void app_main()
{
    beaconMsgQueue = xQueueCreate(60, sizeof(BeaconMsg_t));
    lidar_lite_init();
    alphadisplay_init();
    beacon_rx_init(beaconMsgQueue);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    alphadisplay_write_ascii(0, 'I');
    alphadisplay_write_ascii(1, 'N');
    alphadisplay_write_ascii(2, 'I');
    alphadisplay_write_ascii(3, 'T');

    pulsecounter_init();
    pulsecounter_start();
    lidar_init(LIDAR_FRONT_UART, LIDAR_FRONT);
    lidar_init(LIDAR_BACK_UART, LIDAR_BACK);

#ifdef WIFI_ENABLED // defined in server_communication.h
    wifi_init();
    wifi_connect();
    wifi_wait_for_ip();
    server_comm_init();
    crawler_log("Connected\n");
#endif

    alphadisplay_write_ascii(0, 'C');
    alphadisplay_write_ascii(1, 'L');
    alphadisplay_write_ascii(2, 'B');
    alphadisplay_write_ascii(3, 'R');
    crawler_control_init();
    // crawler_calibrate();
    crawler_steering_set_value(PWM_NEUTRAL_US);
    vTaskDelay(2000/portTICK_PERIOD_MS);

    PID_init();

    crawler_control_start();
    xTaskCreate(update_distance_sensors, "update_sensors",
                4096, NULL, configMAX_PRIORITIES-2, NULL);

    xTaskCreate(crawler_speed_monitor, "crawler_speed_monitor",
                4096, NULL, configMAX_PRIORITIES-2, NULL);
#ifdef WIFI_ENABLED
    xTaskCreate(
        crawler_cmd_recv,
        "crawler_cmd_recv",
        4096,
        NULL, configMAX_PRIORITIES-1, NULL);
#endif

    alphadisplay_write_ascii(0, 'R');
    alphadisplay_write_ascii(1, 'E');
    alphadisplay_write_ascii(2, 'D');
    alphadisplay_write_ascii(3, 'Y');
}
