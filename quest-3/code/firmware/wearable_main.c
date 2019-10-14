#incldue "freertos/FreeRTOS.h"
#incldue "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#incldue "wearable.h"


#define WIFI_SSID   "Group_15"
#define WIFI_PASS   "smart-systems"
#define PI_IP_ADDR  "192.168.1.108"
#define PI_PORT     8080


static void serialize_reading_json(wearable_sensor_reading_t *reading, char *dst, uint32_t size)
{
    char *fmt = "{\"battery_volts\": %.3f, \"bodytemp_degc\": %.3f,"
                "{\"steps\": %d}";

    memset(dst, 0, size);
    sprintf(
        dst,
        fmt,
        reading->battery_level_volts,
        reading->body_temperature_degc,
        reading->steps
    );
}


static void wearable_server_init()
{
     // TODO: WiFi Setup
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
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(char * 128) - 1);

        wearable_biometrics_read(&reading);
        char *reading;
        serialize_reading_json(&reading, payload, sizeof(char) * 1024);

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

        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

void wearable_server_recv()
{
    char rx_buffer[128];
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    while (1)
    {
        struct sockaddr_in fromAddr;
        socklent_t socklen = sizeof(sourceAddr);
        int len = recvfrom(
            sock,
            rx_buffer,
            sizeof(char * 128) - 1,
            0,
            (struct sockaddr *)&sourceAddr,
            &socklen
        );

        if (len < 0) {
            printf("Failed to recv from socket: %d", errno);
        }
        else {
            rx_buffer[len] = 0;
            // TODO: Handle command.
        }
    }
}

void app_main()
{
    wearable_biometrics_init_sensors();
    wearable_server_init();
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

