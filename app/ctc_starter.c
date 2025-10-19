#include "ctc_def.h"
#include "msg_util.h"

void start_comm_traffic_center(const uint8_t curr_proc_id)
{
    start_msg_transmitter();
    start_event_handler_for(curr_proc_id);
}

void start_event_processor(const uint8_t curr_proc_id)
{
    open_inter_comm();
    start_event_handler_for(curr_proc_id);
}
