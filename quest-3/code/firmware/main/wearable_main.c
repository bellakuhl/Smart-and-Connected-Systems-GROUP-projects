#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "wearable.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"


#define WIFI_SSID   "Group_15"
#define WIFI_PASS   "smart-systems"
#define WIFI_MAXIMUM_RETRY  5

#define PI_IP_ADDR  "192.168.1.108"
#define PI_PORT     8080

const int WIFI_CONNECTED_BIT = BIT0;

static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;
static wearable_settings_t settings;

static void serialize_reading_json(
        wearable_sensor_reading_t *reading,
        uint32_t alert_period_seconds,
        char *dst, uint32_t size)
{
    char *fmt = "{\"battery_volts\": %.3f, \"bodytemp_degc\": %.3f,"
                 "\"steps\": %d, \"alert_period_seconds\": %d}";

    memset(dst, 0, size);
    sprintf(
        dst,
        fmt,
        reading->battery_level_volts,
        reading->body_temperature_degc,
        reading->steps,
        alert_period_seconds
    );
}



static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            printf("Got IP: %s\n",
                    ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            {
                if (s_retry_num < WIFI_MAXIMUM_RETRY) {
                    esp_wifi_connect();
                    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                    s_retry_num++;
                    printf("retry to connect to the AP\n");
                }
                printf("connect to the AP fail\n");
                break;
            }
        default:
            break;
    }

    return ESP_OK;
}

static void wait_for_ip()
{
    uint32_t bits = WIFI_CONNECTED_BIT;
    xEventGroupWaitBits(
            s_wifi_event_group,
            bits,
            false,
            true,
            portMAX_DELAY);
}

static void wearable_server_init()
{
    s_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void wearable_server_report_sensors()
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    char addr_str[128];
    char payload[1024];
    wearable_sensor_reading_t reading;

    while (1)
    {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr(PI_IP_ADDR);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PI_PORT);
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(char)*128 - 1);

        wearable_sensors_read(&reading);

        if (!settings.step_sensor_enabled) {
            reading.steps = -1;
        }

        if (!settings.temperature_sensor_enabled) {
            reading.body_temperature_degc = -1;
        }

        if (!settings.battery_sensor_enabled) {
            reading.battery_level_volts = -1;
        }

        serialize_reading_json(
                &reading,
                settings.alert_period_sec,
                payload, sizeof(char) * 1024);

        int err = sendto(
            sock,
            payload,
            strlen(payload),
            0,
            (struct sockaddr *)&destAddr,
            sizeof(destAddr)
        );

        if (err < 0) {
            printf("Error sending sensor reading.\n");
        }

        //printf("Sent message: %s\n", payload);
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}


static void update_settings()
{
    wearable_sensor_set_enable(
            WEARABLE_SENSOR_STEP, settings.step_sensor_enabled);

    wearable_sensor_set_enable(
            WEARABLE_SENSOR_TEMPERATURE, settings.temperature_sensor_enabled);

    wearable_sensor_set_enable(
            WEARABLE_SENSOR_BATTERY, settings.battery_sensor_enabled);

    wearable_schedule_alert(settings.alert_period_sec);

    if (settings.alert_now) {
        wearable_trigger_alert();
        settings.alert_now = 0;
    }
}

static void wearable_init()
{
    wearable_sensors_init();
    wearable_server_init();

    settings.battery_sensor_enabled = 1;
    settings.temperature_sensor_enabled = 1;
    settings.step_sensor_enabled = 1;
    settings.alert_now = 0;
    settings.alert_period_sec = 5;

    update_settings();
}

#define RX_BUFFER_SIZE 128
static void wearable_server_recv()
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    struct sockaddr_in me;
    memset((char *) &me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_port = htons(8080);
    me.sin_addr.s_addr = inet_addr("192.168.1.124");

    if (bind(sock, (struct sockaddr*) &me, sizeof(me))==-1)
        printf("Bind failed\n");

    uint8_t rx_buffer[RX_BUFFER_SIZE];

    while (1)
    {
        struct sockaddr_in fromAddr;
        socklen_t socklen = sizeof(fromAddr);
        printf("Waiting to receive...\n");
        int len = recvfrom(
            sock,
            rx_buffer,
            sizeof(uint8_t)*RX_BUFFER_SIZE - 1,
            0,
            (struct sockaddr *)&fromAddr,
            &socklen
        );

        if (len < 0) {
            printf("Failed to recv from socket: %d", errno);
        }
        else {
            if (len != sizeof(wearable_settings_t)) {
                printf("Ignoring message: %d\n", len);
                continue;
            }
            wearable_settings_t *data = (wearable_settings_t *)rx_buffer;
            printf("Battery: %d, Temp: %d, Step: %d, Alert: %d, Period: %d\n",
                data->battery_sensor_enabled,
                data->temperature_sensor_enabled,
                data->step_sensor_enabled,
                data->alert_now,
                data->alert_period_sec
             );

            settings.battery_sensor_enabled = data->battery_sensor_enabled;
            settings.temperature_sensor_enabled = data->temperature_sensor_enabled;
            settings.step_sensor_enabled = data->step_sensor_enabled;
            settings.alert_period_sec = data->alert_period_sec;
            settings.alert_now = data->alert_now;

            update_settings();
        }
    }
}

void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wearable_init();

    // Don't start socket tasks until
    // we have an IP
    wait_for_ip();
    xTaskCreate(
        wearable_server_report_sensors,
        "report_loop",
        4096,
        NULL,
        configMAX_PRIORITIES - 1,
        NULL
    );

    xTaskCreate(
        wearable_server_recv,
        "receive_loop",
        4096,
        NULL,
        configMAX_PRIORITIES,
        NULL
    );
}

