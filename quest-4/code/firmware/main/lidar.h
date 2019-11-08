#ifndef __T15_LIDAR__
#define __T15_LIDAR__

void lidar_init(int uart_num, int rx_pin);
void lidar_read(int uart_num, uint32_t *dist, uint32_t *strength);
#endif

