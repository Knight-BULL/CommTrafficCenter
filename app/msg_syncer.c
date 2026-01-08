#include "com_def.h"
#include "ctc_def.h"
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/un.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

typedef struct T_event_syncer
{
    bool _is_valid;
    uint16_t _on_event;
    Msg_pkg _msg;
    Handler _handle;
    void* _sync_data;
    uint16_t _len;
    pthread_mutex_t _lock;
    pthread_cond_t _cond;
}Event_syncer;

pthread_mutex_t syncers_lock;

static Event_syncer event_syncers[MAX_MSG_NUM];

struct timespec set_time_out_ms(const uint32_t time_ms)
{
    struct timespec abstime;
    struct timespec tp;
    uint32_t nsec = 0;
    uint32_t timeout_ms = time_ms;

    clock_gettime(CLOCK_MONOTONIC, &tp);
    nsec = tp.tv_nsec + (timeout_ms % 1000) * 1000000;
    abstime.tv_nsec = nsec % 1000000000;
    abstime.tv_sec = tp.tv_sec + nsec/1000000000 + timeout_ms/1000;

    return abstime;
}

void init_syncers()
{
    pthread_mutex_init(&syncers_lock, NULL);

    for (int i=0; i < MAX_MSG_NUM; i++)
    {
        pthread_mutex_init(&(event_syncers[i]._lock),NULL);

        //初始化条件变量(使用 CLOCK_MONOTONIC)
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
        pthread_cond_init(&(event_syncers[i]._cond), &attr);
        pthread_condattr_destroy(&attr);
        
        event_syncers[i]._is_valid = true;
        event_syncers[i]._on_event = INVALID_EVENT;
        event_syncers[i]._handle = NULL;
        event_syncers[i]._len = 0;
        event_syncers[i]._sync_data = NULL;
        (void)memset(&(event_syncers[i]._msg),0x00,sizeof(Msg_pkg));
    }
}

static Event_syncer *alloc_syncer()
{
    pthread_mutex_lock(&syncers_lock);

    for(int i = 0; i < 64; i++)
    {
        if(event_syncers[i]._is_valid)
        {
            event_syncers[i]._is_valid = false;

            pthread_mutex_unlock(&syncers_lock);
            return &event_syncers[i];
        }

    }

    pthread_mutex_unlock(&syncers_lock);
    return NULL;
}

static void regist_event(Event_syncer *syncer, const uint16_t event_id, Handler handle, void* data_buff, const uint16_t len)
{
    ASSERT_NOT_NULL_WITH_ACT(syncer, return, PRT);

    pthread_mutex_lock(&syncers_lock);

    syncer->_on_event = event_id;

    if (NULL != handle)
    {
        syncer->_handle = handle;
    }

    if (NULL != data_buff)
    {
        syncer->_sync_data = data_buff;
    }

    syncer->_len = len;

    pthread_mutex_unlock(&syncers_lock);
}

static int sync_act(Event_syncer *syncer)
{
    if (pthread_mutex_lock(&(syncer->_lock)) != 0)
    {
        return FAILED;
    }

    int ret = FAILED;
    struct timespec time_out = set_time_out_ms(250);

    int ret_wait = pthread_cond_timedwait(&(syncer->_cond), &(syncer->_lock), &time_out);

    if (ret_wait == ETIMEDOUT)
    {
        // TRACE_ERR
        pthread_mutex_unlock(&(syncer->_lock));
        return FAILED;
    }

    const uint16_t data_len = get_msg_len(syncer->_msg.data);

    if (NULL != syncer->_handle)
    {
        ret = syncer->_handle(syncer->_msg.data, data_len);
    }

    pthread_mutex_unlock(&(syncer->_lock));

    return ret;
}

static int sync_data(Event_syncer *syncer)
{
    ASSERT_NOT_NULL(syncer, FAILED, PRT);

    if (pthread_mutex_lock(&(syncer->_lock)) != 0)
    {
        // TRACE_ERR
        return FAILED;
    }

    int ret = FAILED;
    struct timespec time_out = set_time_out_ms(250);
    int ret_wait = pthread_cond_timedwait(&(syncer->_cond), &(syncer->_lock), &time_out);

    if (ret_wait == ETIMEDOUT)
    {
        // TRACE_ERR
        pthread_mutex_unlock(&(syncer->_lock));
        return FAILED;
    }

    if (syncer->_sync_data == NULL)
    {
        // TRACE_ERR
        ret = FAILED;
    }
    else
    {
        (void)memcpy(syncer->_sync_data, syncer->_msg.data, syncer->_len);
        ret = SUCCESS;
    }

    pthread_mutex_unlock(&(syncer->_lock));
    return ret;
}

static void notify_handle(Event_syncer* curr_syncer, const Msg_pkg* sync_msg)
{
    ASSERT_NOT_NULL_WITH_ACT(curr_syncer, return, PRT);
    (void)memcpy(&(curr_syncer->_msg), sync_msg, sizeof(Msg_pkg));

    pthread_mutex_lock(&(curr_syncer->_lock));
    pthread_cond_signal(&(curr_syncer->_cond));
    pthread_mutex_unlock(&(curr_syncer->_lock));
}

static void free_syncer(Event_syncer* curr_syncer)
{
    ASSERT_NOT_NULL_WITH_ACT(curr_syncer, return, PRT);

    pthread_mutex_lock(&syncers_lock);
    curr_syncer->_is_valid = true;
    curr_syncer->_on_event = INVALID_EVENT;
    curr_syncer->_handle = NULL;
    curr_syncer->_len = 0;
    curr_syncer->_sync_data = NULL;
    (void)memset(&curr_syncer->_msg, 0x00, sizeof(Msg_pkg));
    pthread_mutex_unlock(&syncers_lock);
}

int wait_on_with_act(const uint16_t event, Handler func)
{
    Event_syncer *syncer = alloc_syncer();
    ASSERT_NOT_NULL(syncer, FAILED, PRT);

    regist_event(syncer, event, func, NULL, 0);
    int ret =sync_act(syncer);
    free_syncer(syncer);

    return ret;
}

int wait_on(const uint16_t event, void* data, const uint16_t len)
{
    ASSERT_NOT_NULL(data, FAILED, PRT);

    Event_syncer *syncer = alloc_syncer();
    ASSERT_NOT_NULL(syncer, FAILED, PRT);

    regist_event(syncer, event, NULL, data, len);
    int ret=sync_data(syncer);
    free_syncer(syncer);

    return ret;
}

int sync_event(const Msg_pkg* msg)
{
    pthread_mutex_lock(&syncers_lock);

    for(int i = 0; i < MAX_MSG_NUM; i++)
    {
        if(msg->head.text.event == event_syncers[i]._on_event)
        {
            notify_handle(&event_syncers[i],msg);
            // TRACE_INFO(PRT, "sync event[0x%x] SUCCESS!", msg->head.text.event);
            pthread_mutex_unlock(&syncers_lock);
            return SUCCESS;
        }
    }
    // TRACE_INFO(PRT, "event[ox%x] seems not need sync, try handle directly!", msg->l
    pthread_mutex_unlock(&syncers_lock);
    return FAILED;
}
