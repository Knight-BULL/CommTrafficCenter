#ifndef __RS485_UTIL__H__
#define __RS485_UTIL__H__

#include <stdint.h>
#include <fcntl.h>

int open_rs485();
void close_rs485();
void notify_recv();
int send_to_rs485(const void *_buff, const uint16_t _len);
int recv_from_rs485(void *_buff, const uint16_t _len);
int send_to_rs485_sync();

#endif
