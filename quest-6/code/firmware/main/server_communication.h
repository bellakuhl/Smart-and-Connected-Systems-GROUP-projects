#ifndef __T15_SERVER_COMM__
#define __T15_SERVER_COMM__

#define WIFI_ENABLED

void server_comm_init();
int server_log_split_time(
    uint8_t beacon_id,
    float split_time,
    char *response,
    int *response_len,
    int max_response_len);
void crawler_log(const char *format, ...);

#endif