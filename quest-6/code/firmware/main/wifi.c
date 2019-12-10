/* Isabella Kuhl, Laura Reeves, Joesph Roosi
* 11/2/19
* Program for initalizing wifi
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#define WIFI_MAXIMUM_RETRY  5
#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASSPHRASE CONFIG_WIFI_PASSPHRASE

const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "WIFI";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static char *s_wifi_ip = NULL;


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "Got IP: %s\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            s_wifi_ip = ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip);
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            {
                if (s_retry_num < WIFI_MAXIMUM_RETRY) {
                    esp_wifi_connect();
                    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                    s_retry_num++;
                    ESP_LOGW(TAG, "retry to connect to the AP\n");
                }
                ESP_LOGE(TAG, "connect to the AP fail\n");
                break;
            }
        default:
            break;
    }

    return ESP_OK;
}

esp_err_t wifi_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    return ret;
}

void wifi_connect()
{
    s_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSPHRASE
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_wait_for_ip()
{
    uint32_t bits = WIFI_CONNECTED_BIT;
    xEventGroupWaitBits(
            s_wifi_event_group,
            bits,
            false,
            true,
            portMAX_DELAY);
}

char *wifi_get_ip_addr()
{
    return s_wifi_ip;
}

