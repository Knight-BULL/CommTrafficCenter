#include <sys/socket.h>
#include <sys/un.h>
#include "com_def.h"
#include "com_event.h"
#include "msg_util.h"
#include "mq_util.h"
#include "rs485_util.h"
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#define RX_BUFF_LEN ((uint16_t)(2 * sizeof(Msg_pkg)))

const char MAGIC_WORD[2] = {0XAA, 0X55};
const char OLD_MAGIC_WORD[2] = {'S', 'M'};

#define FRAME_HAED_OFFSET           0
#define FRAME_DEV_OFFSET            2
#define FRAME_CARD_OFFSET           3
#define FRAME_CMD_TYPE_OFFSET       4
#define FRAME_CMD_OFFSET            5
#define FRAME_LEN_OFFSET            6
#define FRAME_DATA_OFFSET           7
#define FRAME_HEADER_LEN            7

typedef enum
{
    STEP_RECV_START,

    STEP_RECV_HEAD,
    STEP_RECV_FULL,

    STEP_OLD_CHECK_LEN,
    STEP_OLD_CHECK_FULL,

    STEP_RECV_END,
}RecvStep;

RecvStep next_step;
uint16_t total_recv_len = 0;
uint16_t rx_add_up_len = 0;
char *head_pos = NULL;
char msg_buff[RX_BUFF_LEN];

uint8_t self_pid = INVALID;

uint8_t get_self_pid()
{
    return self_pid;
}

void init_proc_info(const uint8_t pid)
{
    self_pid = pid;
}

static int connect_socket(const char *socket_name)
{
    ASSERT_NOT_NULL(socket_name, -1, PRT);

    int socket_handle = -1;

    if ((socket_handle = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        // TRACE_INFO
        return -1;
    }

    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;

    strcpy(remote.sun_path, socket_name);

    int len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    int conn_count = 0;
    while (connect(socket_handle, (struct sockaddr *)&remote, len) == -1)
    {
        if (++conn_count >= 100)
        {
            break;
        }    

        usleep(10000);
    }

    if (conn_count >= 100)
    {
        // TRACE_INFO
        close(socket_handle);
        return -1;
    }
 
    // TRACE_INFO
    return socket_handle;
}

static char* get_msg_head(const char *text, int idxLen, const char *patn, int patnLen)
{
    ASSERT_NOT_NULL(text, NULL, PRT);
    ASSERT_NOT_NULL(patn, NULL, PRT);

    int idxText,idxPatn;

    if((idxLen < 0) || (patnLen < 0))
    {
        return NULL;
    }
    for(idxText = 0;idxText < idxLen; )
    {
        while((idxText + patnLen)<= idxLen && text[idxText] != patn[0])
        {
            idxText++;
        }
        if((idxText + patnLen)> idxLen)
        {
            return NULL;
        }

        for(idxPatn = 1; idxPatn < patnLen; idxPatn++)
        {
            if(text[idxText + idxPatn] != patn[idxPatn])//check the following element
            {
                break;
            }
        }

        if(idxPatn == patnLen)//match the sub-elements
        {
            return(char *)(text + idxText);
        }
        else
        {
            idxText += idxPatn;
        }
    }

    return NULL;
}

static int recv_raw_data(char *msg_buff, uint16_t *curr_len)
{
    ASSERT_NOT_NULL(msg_buff, FAILED, PRT);

    int rcv_cnt = 0;
    char raw_data[sizeof(Msg_pkg)];
    memset(raw_data, 0, sizeof(raw_data));

    rcv_cnt = recv_from_rs485(raw_data, sizeof(raw_data));//从485缓冲区读出
    if(rcv_cnt > 0)
    {
        if(((*curr_len) + rcv_cnt)> RX_BUFF_LEN)
        {
            // TRACE_ERR(PRT,"recv raw data FAILED");
            return FAILED;
        }
        memcpy(msg_buff + (*curr_len), raw_data, rcv_cnt);
        (*curr_len) += rcv_cnt;
    }

    // TRACE_INFO(PRT, "recv raw data SuccEss" );
    return SUCCESS;
}

static int try_recv_body(char* recv_dst)
{
    if (((msg_buff + rx_add_up_len) - head_pos) >= total_recv_len)//接收到的有效数据长度等于数据自身的总长度，认为数据完备可以完成接受
    {
        (void)memcpy(recv_dst, head_pos, total_recv_len);
        next_step = STEP_RECV_END;

        Msg_pkg *pkg =(Msg_pkg *)recv_dst;
        uint16_t data_len = get_msg_len_from_pkg(pkg);
    // TRACE INFO(PRT, "recv new_msg event[@x%x], total len = %d data lenl%d] SUCCESS!", pkg->head,text,event, msg buff + rx add_up len);
        return SUCCESS;
    }

    next_step = STEP_RECV_FULL;

    // TRACE_INFO(PRT, "recv len = %d CONINUE!", msg buff + rx_add up_len . head pos );
    return CONTINUE;
}

static int try_recv_head(char* recv_dst)
{
    if(((msg_buff + rx_add_up_len)- head_pos)>= sizeof(Msg_head))//接收到的有效数据长度大于等于消息头的长度了
    {
        Msg_head head_tmp;
        (void)memset(&head_tmp, 0x00, sizeof(Msg_head));

        (void)memcpy(head_tmp.buff, head_pos, sizeof(Msg_head));

        total_recv_len = head_tmp.text.total_len;
        if (total_recv_len > sizeof(Msg_pkg))
        {
            // TRACE_INFO(PRT, "total recv len=%d", total recv len);

            next_step = STEP_RECV_START;
            return FAILED;
        }

        ASSERT_CONTINUE(try_recv_body(recv_dst), PRT);
    }

    next_step = STEP_RECV_HEAD;
    return CONTINUE;
}

static int recv_msg_old_flow(char* recv_dst);
static int convert_old_to_new(char* raw_msg);

static int start_recv(char* recv_dst)
{
    head_pos = get_msg_head(msg_buff, rx_add_up_len, MAGIC_WORD, 2);//检查帧头关键字，判断是否有关键字
    if(head_pos != NULL)
    {
        // TRACE_INFO(PRT, "enter new recv flow");
        return try_recv_head(recv_dst);
    }
    else
    {
        head_pos = get_msg_head(msg_buff, rx_add_up_len, OLD_MAGIC_WORD, 2);//检查帧头关键字，判断是否有关键字
        if(head_pos != NULL)
        {
            // TRACE_INFO(PRT, "enter old recv flow");
            next_step = STEP_OLD_CHECK_LEN;
            if(SUCCESS == recv_msg_old_flow(recv_dst))
            {
                return convert_old_to_new(recv_dst);
            }

            return CONTINUE;
        }
    }

    next_step = STEP_RECV_START;
    return CONTINUE;
}

static int try_recv_old_msg_body(char *msg_dst, const uint16_t len)
{
    if (((msg_buff + rx_add_up_len) - head_pos) >= (len + 9))
    {
        int cp_cnt = 0;

        memcpy(msg_dst, head_pos, (len + 9));
        cp_cnt = ((msg_buff + rx_add_up_len) - (head_pos + len + 9));
        memcpy(msg_buff, (head_pos + len +9), cp_cnt);
        memset(msg_buff + cp_cnt, 0, sizeof(msg_buff) - cp_cnt);
        rx_add_up_len = cp_cnt;
        // TRACE INFO(PRT,"recv old_msg 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x x%02x".
        //                 msg_dst[e], msg_dst[1], msg_dst[2], msg dst[3], msg_dst[4], msg_dst[5], msg dst[6]);
        next_step = STEP_RECV_END;
        return SUCCESS;
    }

    return CONTINUE;
}
static int recv_msg_old_flow(char* recv_dst)
{
    static unsigned char msg_len =0;
    switch(next_step)
    {
        case STEP_OLD_CHECK_LEN:
            if(((msg_buff + rx_add_up_len) - head_pos) < 7)
            {
                break;
            }
            msg_len = *(head_pos + FRAME_LEN_OFFSET);
            ASSERT_CONTINUE(try_recv_old_msg_body(recv_dst, msg_len), PRT);
            next_step = STEP_OLD_CHECK_FULL;
            break;

        case STEP_OLD_CHECK_FULL:
            ASSERT_CONTINUE(try_recv_old_msg_body(recv_dst, msg_len), PRT);
            break;
        
        default:
            break;
    }

    return CONTINUE;
}

static int convert_old_to_new(char *raw_msg)
{
    if(raw_msg[FRAME_LEN_OFFSET] > MAX_EXT_MSG_LEN)
    {
        return FAILED;
    }

    Msg_pkg format_msg;
    uint8_t check_sum_offset =0;
    (void)memset(&format_msg, 0x00,sizeof(format_msg));
    format_msg.head.text.magic[0] = MAGIC_WORD[0];
    format_msg.head.text.magic[1] = MAGIC_WORD[1];
    format_msg.head.text.event = raw_msg[FRAME_CMD_OFFSET];
    format_msg.head.text.cmd_type_old = raw_msg[FRAME_CMD_TYPE_OFFSET];
    format_msg.head.text.dst_dir.shelf = raw_msg[FRAME_DEV_OFFSET];
    format_msg.head.text.dst_dir.card = raw_msg[FRAME_CARD_OFFSET];
    format_msg.head.text.dst_dir.pid = get_pid_by_event(format_msg.head.text.event);
    format_msg.head.text.total_len = raw_msg[FRAME_LEN_OFFSET] + sizeof(format_msg.head) + sizeof(format_msg.fcs);
    format_msg.head.text.protocol_type = OLD_PTP;

    check_sum_offset = FRAME_DATA_OFFSET + raw_msg[FRAME_LEN_OFFSET];
    format_msg.fcs = raw_msg[check_sum_offset];// 用fcs来存放老版本消息的checksum

    (void)memcpy(format_msg.data, &raw_msg[FRAME_DATA_OFFSET], raw_msg[FRAME_LEN_OFFSET]);
    (void)memset(raw_msg, 0x00, sizeof(Msg_pkg));

    (void)memcpy(raw_msg, &format_msg, format_msg.head.text.total_len);

    return SUCCESS;
}

static unsigned char check_485fcs(char *data, int len)
{
    unsigned char fcs = 0;
    int i;

    for(i=0; i < len; i++)
    {
        fcs += data[i];
    }

    return fcs;
}

static uint16_t convert_new_to_old(const Msg_pkg* new_msg, char* old_msg)
{
    uint16_t msg_len = 0;

    ASSERT_NOT_NULL(old_msg, msg_len, PRT);
    ASSERT_NOT_NULL(new_msg, msg_len, PRT);

    old_msg[FRAME_HAED_OFFSET] = 'S';
    old_msg[FRAME_HAED_OFFSET + 1] = 'M';
    old_msg[FRAME_DEV_OFFSET] = new_msg->head.text.dst_dir.shelf;
    old_msg[FRAME_CARD_OFFSET] = new_msg->head.text.dst_dir.card;
    old_msg[FRAME_CMD_TYPE_OFFSET]= new_msg->head.text.cmd_type_old;
    old_msg[FRAME_CMD_OFFSET] = new_msg->head.text.event;
    old_msg[FRAME_LEN_OFFSET] = get_msg_len_from_pkg(new_msg);
    (void)memcpy(&old_msg[FRAME_DATA_OFFSET], new_msg->data, old_msg[FRAME_LEN_OFFSET]);

    old_msg[FRAME_HEADER_LEN + old_msg[FRAME_LEN_OFFSET]] = check_485fcs(old_msg + 2, old_msg[FRAME_LEN_OFFSET] + 3 + 2);
    old_msg[FRAME_HEADER_LEN + old_msg[FRAME_LEN_OFFSET] + 1] = '@';
    msg_len = old_msg[FRAME_LEN_OFFSET]+ FRAME_HEADER_LEN + 2;//头+数据+fcs+尾

    return msg_len;
}

static int try_recv_full_msg(char *raw_msg)
{
    ASSERT_NOT_NULL(raw_msg, FAILED, PRT);

    total_recv_len = 0;
    rx_add_up_len = 0;
    head_pos = NULL;
    next_step = STEP_RECV_START;

    (void)memset(msg_buff, 0x00, sizeof(msg_buff));

    while (1)
    {
        switch (next_step)
        {
            case STEP_RECV_START:
                ASSERT_CONTINUE(start_recv(raw_msg), PRT);
                break;
        
            case STEP_RECV_HEAD:
                ASSERT_CONTINUE(try_recv_head(raw_msg), PRT);
                break;

            case STEP_RECV_FULL:
                ASSERT_CONTINUE(try_recv_body(raw_msg), PRT);
                break;     

            case STEP_OLD_CHECK_LEN:
            case STEP_OLD_CHECK_FULL:
                if (SUCCESS == recv_msg_old_flow(raw_msg))
                {
                    return convert_old_to_new(raw_msg);
                }
                break;

            default:
                break;
        }

        if (SUCCESS != recv_raw_data(msg_buff, &rx_add_up_len))
        {
            return FAILED;
        }
    }

    return FAILED;
}

static int msg_check(char *_msg)
{
    Msg_pkg msg = {0};
    uint8_t curr_card_id = 0x00;

    curr_card_id = get_self_card_id();

    if (curr_card_id != (msg.head.text.dst_dir.card + 2))
    {
        // TRACE_ERR
        return FAILED;
    }

    // TRACE_INFO()

    if (msg.head.text.protocol_type == OLD_PTP)
    {
        uint8_t calc_sum = 0, data_len = 0;
        calc_sum += msg.head.text.dst_dir.shelf;
        calc_sum += msg.head.text.dst_dir.card;
        calc_sum += msg.head.text.cmd_type_old;
        calc_sum += msg.head.text.event;

        data_len = msg.head.text.total_len - sizeof(msg.head) - sizeof(msg.fcs);
        calc_sum += data_len;
        for (int i = 0; i < data_len; i++)
        {
            calc_sum += msg.data[i];
        }

        if (calc_sum != msg.fcs)
        {
            // TRACE_ERR
            return FAILED;
        }
    }
    else
    {
        uint8_t fcs_result = 0;
        fcs_result = calc_msg_xor(&msg);

        if (fcs_result != msg.fcs)
        {
            // TRACE_ERR
            return FAILED;
        }
    }

    return SUCCESS;
}

int recv_msg_from_exter(Msg_pkg *msg)
{
    ASSERT_NOT_NULL(msg, FAILED, PRT);

    char rcv_buf[sizeof(Msg_pkg)];
    (void)memset(rcv_buf, 0x00, sizeof(rcv_buf));

    ASSERT_SUCC_WITH(try_recv_full_msg(rcv_buf), FAILED, PRT);
    ASSERT_SUCC_WITH(msg_check(rcv_buf), FAILED, PRT);

    (void)memcpy(msg, rcv_buf, sizeof(Msg_pkg));

    return SUCCESS;
}

int recv_msg_from_inter(void *data, const uint16_t len)
{
    return recv_msg_from_mq(data, len);
}

int send_msg_to_local(const uint8_t _pid, void * data, const uint16_t len)
{
    int dst_socket = 0;

    char *dst_name = get_comm_info_from_matrix(_pid);
    ASSERT_NOT_NULL(dst_name, FAILED, PRT);

    dst_socket = connect_socket(dst_name);
    ASSERT_FALSE(dst_socket == -1, FAILED, PRT);

    if (send(dst_socket, data, len, MSG_NOSIGNAL) == -1)
    {
        // TRACE_ERR

        close(dst_socket);
        return FAILED;
    }

    close(dst_socket);
    return SUCCESS;
}

int send_msg_to_remote(const Msg_pkg *_msg, const uint16_t _len)
{
    ASSERT_NOT_NULL(_msg, FAILED, PRT);

    uint16_t send_len = _len;

    char send_msg[sizeof(Msg_pkg)];
    (void)memset(send_msg, 0x00, sizeof(send_msg));

    if (OLD_PTP == _msg->head.text.protocol_type)
    {
        send_len = convert_new_to_old(_msg, send_msg);
    }
    else
    {
        (void)memcpy(send_msg, _msg, sizeof(send_msg));
    }

    if (NO_ACK == _msg->head.text.ack_flag)
    {
        if (SUCCESS == send_to_rs485(send_msg, send_len))
        {
            // TRACE_INFO
            return SUCCESS;
        }

        // TRACE_ERR
        return FAILED;
    }

    if (SUCCESS == send_to_rs485_sync(send_msg, send_len))
    {
        // TRACE_INFO
        return SUCCESS;
    }

    // TRACE_ERR
    return FAILED;
}

bool is_cross_dev(const uint8_t dst_shelf_no, const uint8_t dst_card_no)
{
    if ((dst_shelf_no == get_self_shelf_id()) && (dst_card_no == get_self_card_id()))
    {
        return false;
    }

    return true;
}

uint8_t calc_msg_xor(const Msg_pkg *msg)
{
    uint8_t xor = 0;
    for (int i = 0; i < sizeof(msg->head); i++)
    {
        xor ^= msg->head.buff[i];
    }

    for (int i = 0; i < (msg->head.text.total_len - sizeof(msg->fcs) - sizeof(msg->head)); i++)
    {
        xor ^= msg->data[i];
    }

    return xor;
}

void package_msg(Msg_pkg *_msg_buff, const uint16_t _event_id, const Msg_direction *_dst_dir, \
                 const void *_data_tmp, const uint16_t _data_len, const uint8_t _is_ack)
{
    ASSERT_NOT_NULL_WITH_ACT(_msg_buff, return, PRT);
    ASSERT_NOT_NULL_WITH_ACT(_dst_dir, return, PRT);

    _msg_buff->head.text.magic[0] = MAGIC_WORD[0];
    _msg_buff->head.text.magic[1] = MAGIC_WORD[1];

    _msg_buff->head.text.event = _event_id;

    _msg_buff->head.text.ack_flag = _is_ack;

    _msg_buff->head.text.dst_dir.shelf = _dst_dir->shelf;
    _msg_buff->head.text.dst_dir.card = _dst_dir->card;
    _msg_buff->head.text.dst_dir.pid = _dst_dir->pid;


    _msg_buff->head.text.src_dir.shelf = get_self_shelf_id();
    _msg_buff->head.text.src_dir.card = get_self_card_id();
    _msg_buff->head.text.src_dir.pid = get_self_pid();

    if (_data_tmp)
    {
        (void)memcpy(_msg_buff->data, _data_tmp, _data_len);
    }

    _msg_buff->head.text.total_len = sizeof(Msg_head) + sizeof(_msg_buff->fcs) + _data_len;

    _msg_buff->fcs = calc_msg_xor(_msg_buff);
}

void package_new_ptp_msg(Msg_pkg *msg_buff, const uint16_t event_id, const Msg_direction *dst_dir, \
                         const void *data_tmp, const uint16_t data_len, const uint8_t is_ack)
{
    package_msg(msg_buff, event_id, dst_dir, data_tmp, data_len, is_ack);

    msg_buff->head.text.protocol_type = NEW_PTP;
}

void package_old_ptp_msg(Msg_pkg *msg_buff, const uint16_t event_id, const Msg_direction *dst_dir, \
                         const void *data_tmp, const uint16_t data_len, const uint8_t cmd_type, const uint8_t is_ack)
{
    package_msg(msg_buff, event_id, dst_dir, data_tmp, data_len, is_ack);

    msg_buff->head.text.protocol_type = OLD_PTP;
    msg_buff->head.text.cmd_type_old = cmd_type;
}

int transmit_msg(const Msg_pkg *_msg, const uint16_t _len)
{
    return send_msg_to_mq(_msg, _len);
}

int open_exter_comm()
{
    return open_rs485();
}

int open_inter_comm()
{
    return open_msg_queue(sizeof(Msg_pkg));
}

void close_exter_comm()
{
    close_rs485();
}

void close_inter_comm()
{
    close_msg_queue();
}

void free_exter_comm()
{
    notify_recv();
}

Msg_direction get_src_info(const char *_data)
{
    if (NULL == _data)
    {
        Msg_direction dir_err;
        dir_err.shelf = INVALID;
        dir_err.card = INVALID;
        dir_err.pid = INVALID;

        return dir_err;
    }

    Msg_pkg *msg = container_of(_data, Msg_pkg, data);

    return msg->head.text.src_dir;
}

Msg_direction get_dst_info(const char *_data)
{
    if (NULL == _data)
    {
        Msg_direction dir_err;
        dir_err.shelf = INVALID;
        dir_err.card = INVALID;
        dir_err.pid = INVALID;

        return dir_err;
    }

    Msg_pkg *msg = container_of(_data, Msg_pkg, data);
    return msg->head.text.dst_dir;
}

uint16_t get_event_id(const char *_data)
{
    ASSERT_NOT_NULL(_data, INVALID_EVENT, PRT);

    Msg_pkg *msg = container_of(_data, Msg_pkg, data);
    return msg->head.text.event;
}

uint8_t get_old_cmd_type(const char *_data)
{
    ASSERT_NOT_NULL(_data, INVALID_EVENT, PRT);

    Msg_pkg *msg = container_of(_data, Msg_pkg, data);
    return msg->head.text.cmd_type_old;
}

Msg_head get_event_head(const char *_data)
{
    if (NULL == _data)
    {
        Msg_head head_tmp;
        memset(&head_tmp, 0xFF, sizeof(head_tmp));

        return head_tmp;
    }

    Msg_pkg *msg = container_of(_data, Msg_pkg, data);
    return msg->head;
}

uint16_t get_msg_len(const char *_data)
{
    ASSERT_NOT_NULL(_data, 0, PRT);

    Msg_pkg *msg = container_of(_data, Msg_pkg, data);
    uint16_t data_len = msg->head.text.total_len - sizeof(msg->head) - sizeof(msg->fcs);

    return data_len;
}

uint16_t get_msg_len_from_pkg(const Msg_pkg *_pkg)
{
    ASSERT_NOT_NULL(_pkg, 0, PRT);

    uint16_t data_len = _pkg->head.text.total_len - sizeof(_pkg->head) - sizeof(_pkg->fcs);

    return data_len;
}

int try_create_socket(const char *sockname, int nconn)
{
    ASSERT_NOT_NULL(sockname, FAILED, PRT);

    int socket_handle = 0;
    struct sockaddr_un local;

    if ((socket_handle = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        // TRACE_ERR
        return -1;
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, sockname);
    unlink(local.sun_path);
    int len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(socket_handle, (struct sockaddr *)&local, len) == -1)
    {
        // TRACE_ERR
        return -1;
    }

    if (listen(socket_handle, nconn) == -1)
    {
        // TRACE_ERR
        return -1;
    }
    
    return socket_handle;
}