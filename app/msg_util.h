#ifndef __MSG_UTIL__H__
#define __MSG_UTIL__H__

#include <stdint.h>
#include <stdbool.h>
#include "ctc_def.h"

char *get_comm_info_from_matrix(const uint8_t proc_id);

bool is_cross_dev(const uint8_t dst_shelf_no, const uint8_t dst_card_no);

uint8_t calc_msg_xor(const Msg_pkg *msg);
void package_msg(Msg_pkg *_msg_buff, const uint16_t _event_id, const Msg_direction *_dst_dir, \
                 const void *_data_tmp, const uint16_t _data_len, const uint8_t _is_ack);

void package_new_ptp_msg(Msg_pkg *msg_buff, const uint16_t event_id, const Msg_direction *dst_dir, \
                         const void *data_tmp, const uint16_t data_len, const uint8_t is_ack);

void package_old_ptp_msg(Msg_pkg *msg_buff, const uint16_t event_id, const Msg_direction *dst_dir, \
                         const void *data_tmp, const uint16_t data_len, const uint8_t cmd_type, const uint8_t is_ack);
int transmit_msg(const Msg_pkg *_msg, const uint16_t _len);
int recv_msg_from_exter(Msg_pkg *msg);
int recv_msg_from_inter(void *data, const uint16_t len);
int send_msg_to_local(const uint8_t _pid, void * data, const uint16_t len);
int send_msg_to_remote(const Msg_pkg *_msg, const uint16_t _len);
int open_exter_comm();
int open_inter_comm();
void close_exter_comm();
void close_inter_comm();
void free_exter_comm();
Msg_direction get_src_info(const char *_data);
Msg_direction get_dst_info(const char *_data);
uint16_t get_event_id(const char *_data);
uint8_t get_old_cmd_type(const char *_data);
Msg_head get_event_head(const char *_data);
uint16_t get_msg_len(const char *_data);
uint16_t get_msg_len_from_pkg(const Msg_pkg *_pkg);
int try_create_socket(const char *sockname, int nconn);

#endif
