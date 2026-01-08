#ifndef __MSG_DEF__H__
#define __MSG_DEF__H__

#include <stdint.h>
#include "../utils/com_def.h"

#define NEED_ACK        ((uint8_t)1)
#define NO_ACK          ((uint8_t)0)

#define SOCKET_NCONN    ((uint8_t)5)

#define INVALID         ((uint8_t)0XFF)
#define INVALID_EVENT   INVALID_UINT16

#define MAX_BODY_LEN    ((uint16_t)4096)
#define MAX_EXT_MSG_LEN ((uint16_t)256)

#define OLD_PTP         ((uint8_t)1)
#define NEW_PTP         ((uint8_t)0)

void ack_for_old_set_msg(const char* data_, const int8_t ack_status);

#define EVENT_PROCESS_BEGIN \
    int process_event(const uint16_t _event_id, const char* _data, const uint16_t _len) \
    {   \
        int ret = 0;\
        switch(_event_id)\
        {

#define PROCESS_EVENT(_event, _handle)\
        case _event:\
            ret = _handle(_data, _len);\
            break;      

#define EVENT_PROCESS_END\
        default:\
            return FAILED;\
        }\
        ack_for_old_set_msg(_data, ret);\
        return SUCCESS;\
    }

typedef enum E_proc_list
{
    REF_SEL_PROC = 100,
    GNSS_PROC,
}Proc_list;

typedef struct T_msg_direction
{
    uint8_t shelf;
    uint8_t card;
    uint8_t pid;
}Msg_direction;

typedef struct 
{
    char magic[2];
    uint16_t total_len;
    uint16_t event;
    uint8_t ack_flag;
    uint8_t cmd_type_old;
    uint8_t protocol_type;
    Msg_direction src_dir;
    Msg_direction dst_dir;
}T_msg_head;

typedef union U_msg_head
{
    char buff[sizeof(T_msg_head)];
    T_msg_head text;
}Msg_head;

typedef struct T_msg_entity
{
    Msg_head head;
    uint8_t fcs;
    char data[MAX_BODY_LEN];
}Msg_pkg;

typedef struct T_mailbox
{
    Msg_pkg msg_queue[MAX_MSG_NUM];
    int front;
    int rear;
}Mailbox;

typedef int (*Handler)(void* _data, const uint16_t _len);

int ctc_send_msg(const uint16_t event, const Msg_direction* dst_dir, \
                 const void* data, const uint16_t len, const uint8_t ack_flag);

int ctc_send_inter_msg(const uint16_t event, const uint8_t _pid, const void* data, \
                       const uint16_t len, const uint8_t ack_flag);                 

int ctc_send_old_msg(const uint16_t event, const Msg_direction* dst_dir, const void* data, \
                     const uint16_t len, const uint8_t cmd_type ,const uint8_t is_need_ack);

int ctc_send_sync_msg(const uint16_t event, const Msg_direction* dst_dir, \
                      const void* data, const uint16_t len);

int ctc_send_sync_msg(const uint16_t event, const Msg_direction* dst_dir, \
                      const void* data, const uint16_t len);

void init_proc_info(const uint8_t pid);

void* exter_transmitter(void* arg);
void* inter_transmitter(void* arg);
void start_msg_transmitter();

void* event_poster(void* arg);
void* event_handler(void* arg);
void start_event_handler_for(const uint8_t curr_proc_id);
void start_event_processor(const uint8_t curr_proc_id);
void start_comm_traffic_center(const uint8_t curr_proc_id);

int wait_on(const uint16_t event, void* data, const uint16_t len);
int wait_on_with_act(const uint16_t event, Handler func);

Msg_direction get_src_info(const char *_data);
Msg_direction get_dst_info(const char *_data);
uint16_t get_event_id(const char *_data); 
uint8_t get_old_cmd_type(const char *_data);
uint8_t get_ptp_type(const char *_data);
Msg_head get_event_head(const char *_data);
uint16_t get_msg_len(const char *_data);
uint16_t get_msg_len_from_pkg(const Msg_pkg *_pkg);

uint8_t get_self_shelf_id();
uint8_t get_self_card_id();
uint8_t get_self_pid();

#endif