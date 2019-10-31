#ifndef __EC444T15_WIFI__
#define __EC444T15_WIFI__
#include <string.h>

esp_err_t wifi_init();
void wifi_connect(
        uint8_t *ssid,
        size_t ssid_size,
        uint8_t *passphrase,
        size_t passphrase_size);
void wifi_wait_for_ip();
char *wifi_get_ip_addr();

#endif

