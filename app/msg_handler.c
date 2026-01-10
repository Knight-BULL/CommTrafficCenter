#include "ctc_def.h"
#include "com_def.h"
#include "msg_util.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "ctc_trace.h"
#include "assert.h"

pthread_t event_poster_thread;
pthread_t event_handler_thread;

extern int process_event(const uint16_t _event_id, const char *_data, const uint16_t _len);
void init_msg_mailbox();
void wait_post();
bool deliver_mail(const Msg_pkg* msg);
bool fetch_mail(Msg_pkg* msg);
void show_mailbox();
bool is_mailbox_empty();
int sync_event(const Msg_pkg* msg);
void init_syncers();

static inline void post_event(const Msg_pkg* msg)
{
    if(SUCCESS == sync_event(msg))
    {
        return;
    }

    deliver_mail(msg);
}

void *event_poster(void *arg)
{
    init_msg_mailbox();
    Msg_pkg recv_buf;
    (void)memset(&recv_buf, 0x00, sizeof(recv_buf));

    int socket_id = 0;
    socklen_t sockaddr_size = sizeof(struct sockaddr);
    int recv_size = 0;
    int socket_local = 0;
    struct sockaddr_un remote;

    char* dst_name = get_comm_info_from_matrix(get_self_pid());
    ASSERT_NOT_NULL(dst_name, NULL, PRT);

    socket_local = try_create_socket(dst_name, SOCKET_NCONN);
    ASSERT_FALSE_WITH_ACT(socket_local < 0, return NULL, PRT);

    while (1)
    {
        if ((socket_id = accept(socket_local, (struct sockaddr *)&remote, &sockaddr_size)) == -1)
        {
            continue;
        }

        (void)memset(&recv_buf, 0x00, sizeof(recv_buf));
        recv_size = recv(socket_id, &recv_buf, sizeof(recv_buf), 0);

        if (recv_size <= 0)
        {
            close(socket_id);
            continue;
        }

        CTC_TRACE_INFO(TRC, "recv event[0x%x] src shelf is [0x%x] src pid is [0x%x]", \
                        recv_buf.head.text.event, recv_buf.head.text.src_dir.shelf,\
                        recv_buf.head.text.src_dir.card, recv_buf.head.text.src_dir.pid);

        post_event(&recv_buf);

        show_mailbox();

        close(socket_id);
    }  
}

void *event_handler(void* arg)
{
    init_syncers();

    while (1)
    {
        if (is_mailbox_empty())
        {
            wait_post();
        }

        Msg_pkg recv_buf;
        (void)memset(&recv_buf, 0x00, sizeof(recv_buf));

        if (fetch_mail(&recv_buf))
        {
            int msg_len = recv_buf.head.text.total_len - sizeof(recv_buf.head) - sizeof(recv_buf.fcs);
            if (SUCCESS != process_event(recv_buf.head.text.event, recv_buf.data, msg_len))
            {
                CTC_TRACE_ERRO(TRC, "process_event[0x%x] len[%d] FAILED!", recv_buf.head.text.event, msg_len);
            }
            else
            {
                CTC_TRACE_INFO(TRC, "process_event[0x%x] len[%d] SUCCED!", recv_buf.head.text.event, msg_len);
            }
        }
    }
}

void start_event_handler_for(const uint8_t proc_id)
{
    init_proc_info(proc_id);

    if (pthread_create(&event_poster_thread, NULL, event_poster, NULL) != 0)
    {
        CTC_TRACE_ERRO(TRC, "create event_poster failed!");
    }

    if (pthread_create(&event_handler_thread, NULL, event_handler, NULL) != 0)
    {
        CTC_TRACE_ERRO(TRC, "create event_handler failed!");
    }
}