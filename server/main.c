#include <stdio.h>
#include "ctc_def.h"
#include "com_def.h"
#include "assert.h"
#include "ctc_trace.h"

int main()
{
    CTC_TRACE_INFO(TRC, "start_msg_transmitter!!!");
    
    start_msg_transmitter();
    while (1)
    {
        CTC_TRACE_INFO(TRC, "do nothing");
        sleep(1);
    }
    
    return 0;
}
