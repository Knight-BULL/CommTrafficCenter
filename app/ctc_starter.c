#include "ctc_def.h"
#include "msg_util.h"
#include "ctc_trace.h"

void start_comm_traffic_center(const uint8_t curr_proc_id)//��ctc�����ã���������ת��
{
    start_msg_transmitter();
}

void start_event_processor(const uint8_t curr_proc_id)//������ҵ������ã��������ݴ���
{
    open_inter_comm();
    start_event_handler_for(curr_proc_id);
}
