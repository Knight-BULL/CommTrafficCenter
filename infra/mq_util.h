#ifndef __MQ_UTIL__H__
#define __MQ_UTIL__H__

#include <stdint.h>

int open_msg_queue(const uint32_t msg_size);
void close_msg_queue();
int send_msg_to_mq(const void *data, const uint16_t len);
int recv_msg_from_mq(void *_data, const uint16_t _len);

#endif
