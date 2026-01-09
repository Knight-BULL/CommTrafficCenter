#include "com_def.h"
#include "ctc_def.h"
#include "msg_util.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "ctc_trace.h"

pthread_t inter_transmitter_thread;
pthread_t exter_transmitter_thread;

void *inter_transmitter(void *arg)
{
    if (FAILED == open_exter_comm())
    {
        // CTC_TRACE_ERRO
    }

    while (1)
    {
        Msg_pkg msg_tmp;
        (void)memset(&msg_tmp, 0x00, sizeof(msg_tmp));

        if (SUCCESS == recv_msg_from_exter(&msg_tmp))
        {
            // CTC_TRACE_INFO
            const uint8_t dst_proc = msg_tmp.head.text.dst_dir.pid;
            send_msg_to_local(dst_proc, &msg_tmp, sizeof(msg_tmp));
            free_exter_comm();
        }
    }

    close_exter_comm();
    return NULL;
}

void *exter_transmitter(void *arg)
{
    if (FAILED == open_inter_comm())
    {
        CTC_TRACE_ERRO(TRC, "open_inter_comm FAILED!!");
    }

    CTC_TRACE_INFO(TRC, "open_inter_comm SUCCED!!");
    while (1)
    {
        Msg_pkg msg_tmp;
        (void)memset(&msg_tmp, 0x00, sizeof(msg_tmp));

        if (SUCCESS == recv_msg_from_inter(&msg_tmp, sizeof(msg_tmp)))
        {
            CTC_TRACE_INFO(TRC, "recv inter event[0x%x] len[%d] SUCCESS!", msg_tmp.head.text.event, get_msg_len_from_pkg(&msg_tmp));
            send_msg_to_remote(&msg_tmp, msg_tmp.head.text.total_len);
        }
    }

    close_inter_comm();
    return NULL;
}

void start_msg_transmitter()
{
    if (pthread_create(&inter_transmitter_thread, NULL, inter_transmitter, NULL) != 0)
    {
        CTC_TRACE_ERRO(TRC, "****** start inter_transmitter FAILED!********");
    }

    if (pthread_create(&exter_transmitter_thread, NULL, exter_transmitter, NULL) != 0)
    {
        CTC_TRACE_ERRO(TRC, "****** start exter_transmitter FAILED!********");
    }
}
