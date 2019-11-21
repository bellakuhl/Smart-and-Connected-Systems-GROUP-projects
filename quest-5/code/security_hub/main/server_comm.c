#include <string.h>
#include "esp_http_client.h"

#define SERVER_HOST CONFIG_SERVER_HOST
#define SERVER_PORT CONFIG_SERVER_PORT

#define SERVER_USERNAME CONFIG_SERVER_USERNAME
#define SERVER_PASSWD CONFIG_SERVER_PASSWORD

#define ERR_CHECKOK(code) if ((code) != 0) {free(body); return -1;}

int serialize_json(int fob_id, int fob_code, char **dst)
{
    char *fmt = "{\"fob_id\":%d,\"fob_code\":%d}";
    size_t strsize = sizeof(char) * strlen(fmt) + 30;
    *dst = (char *)malloc(strsize);
    memset(*dst, 0, strsize);
    if (*dst == NULL) {
        return -1; // malloc failed, handle error?
    }

    sprintf(*dst, fmt, fob_id, fob_code);
    return strlen(*dst);
}

int server_comm_make_request(int fob_id, int fob_code)
{
    esp_http_client_config_t config = {
        .host = SERVER_HOST,
        .port = SERVER_PORT,
        .path = "/fob-access",
        .username = SERVER_USERNAME,
        .password = SERVER_PASSWD,
        .auth_type = HTTP_AUTH_TYPE_BASIC
    };

    esp_http_client_handle_t client;
    client = esp_http_client_init(&config);
    if (client == NULL) {
        return -1;
    }

    char *body;
    int body_size = serialize_json(fob_id, fob_code, &body);
    if (body_size < 0) {
        return body_size;
    }
    printf("Body: %s\n", body);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, body, body_size);
    esp_http_client_perform(client);

    int status_code = esp_http_client_get_status_code(client);
    free(body);

    return status_code;
}

