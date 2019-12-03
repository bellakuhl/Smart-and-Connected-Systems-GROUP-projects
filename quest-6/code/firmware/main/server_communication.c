/* Isabella Kuhl, Laura Reeve, Joseph Rossi
* Module for HTTP communication with the web server
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "esp_http_client.h"

#define SERVER_HOST CONFIG_SERVER_HOST
#define SERVER_HTTP_PORT CONFIG_SERVER_HTTP_PORT
#define SERVER_UDP_PORT CONFIG_SERVER_UDP_PORT

#define SERVER_USERNAME CONFIG_CRAWLER_USERNAME
#define SERVER_PASSWD CONFIG_CRAWLER_PASSWORD

static int g_udp_send_socket;
static const char *TAG = "Crawler";

void server_comm_init()
{
    g_udp_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
}

int serialize_json(char *event, char **dst)
{
    char *fmt = "{\"event\":%d}";
    size_t strsize = sizeof(char) * strlen(fmt) + 30;
    *dst = (char *)malloc(strsize);
    memset(*dst, 0, strsize);
    if (*dst == NULL) {
        return -1; // malloc failed, handle error?
    }

    sprintf(*dst, fmt,  event);
    return strlen(*dst);
}

int server_log_event(char *event, char *response, int *response_len, int max_response_len)
{
    esp_http_client_config_t config = {
        .host = SERVER_HOST,
        .port = SERVER_HTTP_PORT,
        .path = "/cralwer-event/log",
        .username = SERVER_USERNAME,
        .password = SERVER_PASSWD,
        .auth_type = HTTP_AUTH_TYPE_BASIC
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        return -1;
    }

    char *body;
    int body_size = serialize_json(event, &body);
    if (body_size < 0) {
        return body_size;
    }
    ESP_LOGI(TAG, "Request Body: %s\n", body);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, body, body_size);
    esp_http_client_perform(client);

    int content_length = esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);
    if (response != NULL &&  content_length > 0 && content_length <= max_response_len) {
        *response_len = esp_http_client_read(client, response, content_length);
        if (*response_len <= 0) {
            ESP_LOGE(TAG, "Error read data");
        }

        response[*response_len] = 0;
        ESP_LOGD(TAG, "reponse_len = %d", *response_len);
    }

    free(body);
    esp_http_client_cleanup(client);

    return status_code;
}

static void server_send_log(const char *msg)
{
    char addr_str[128];
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = inet_addr(SERVER_HOST);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SERVER_UDP_PORT);
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(char)*128 - 1);

    int err = sendto(
        g_udp_send_socket,
        msg,
        strlen(msg),
        0,
        (struct sockaddr *)&destAddr,
        sizeof(destAddr)
    );

    if (err < 0) {
        ESP_LOGE(TAG, "Error sending message: %d\n", err);
    }
}

void crawler_log(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    ESP_LOGI(TAG, "%s", buffer);
#ifdef WIFI_ENABLED
    server_send_log(buffer);
#endif
}
