#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi.h"
#include "server_comm.h"


void app_main()
{
    wifi_init();
    wifi_connect();
    wifi_wait_for_ip();
    char *ip = wifi_get_ip_addr();
    int status = server_comm_make_request(1234, 6789);
    printf("Status: %d\n", status);
}

