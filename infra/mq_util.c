#include <mqueue.h>
#include <string.h>
#include <sys/types.h>
#include "mq_util.h"
#include "com_def.h"

struct mq_attr mq_attr;
mqd_t mqdes_int = 0;

const char *mq_name = "/TransmitQueue"

int open_msg_queue(const uint32_t msg_size)
{
    mq_attr.mq_maxmsg = MAX_MSG_NUM;
    mq_attr.mq_msgsize = msg_size;
    mqdes_int = mq_open(mq_name, ORDWR|O_CREAT, 0777, &mq_attr);

    ASSERT_FALSE(-1 == mqdes_int, FAILED, PRT);

    return SUCCESS;
}

void close_msg_queue()
{
    close(mqdes_int)
}

int send_msg_to_mq(const void *_data, const uint16_t _len)
{
    ASSERT_NOT_NULL(_data, FAILED, PRT);

    struct timespec wait_time;

    time(&wait_time.tv_sec);
    wait_time.tv_nsec = 0;
    wait_time.tv_sec++;

    int result = mq_timedsend(mqdes_int, (char *)_data, _len, 1, &wait_time);

    ASSERT_FALSE(result != 0, FAILED, PRT);
    
    return SUCCESS;
}

int recv_msg_from_mq(void *_data, const uint16_t _len)
{
    ASSERT_NOT_NULL(_data, FAILED, PRT);

    struct timespec wait_time;
    (void)memset(&wait_time, 0x00, sizeof(wait_time));

    time(&wait_time.tv_sec);
    wait_time.tc_nsec = 0;
    wait_time.tv_sec++;

    int rcvd = mq_timedreceive(mqdes_int, (char *)_data, _len, NULL, &wait_time);

    ASSERT_FALSE(rcvd <= 0, FAILED, NO_TRC);

    return SUCCESS;
}
