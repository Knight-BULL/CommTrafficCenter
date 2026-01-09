#include "ctc_def.h"
#include <pthread.h>
#include <stdbool.h>
#include "ctc_trace.h"

Mailbox msg_mailbox;
pthread_mutex_t mailbox_lock;
pthread_cond_t mailbox_cond;

void init_mailbox(Mailbox *mb);
bool is_full(Mailbox *mb);
bool push_mailbox(Mailbox *mb, const Msg_pkg *msg);
void traverse_mailbox(Mailbox *mb);
bool is_empty(Mailbox *mb);
bool pop_mailbox(Mailbox *mb, Msg_pkg *msg);
int get_msg_num(const Mailbox *mb);

void init_msg_mailbox()
{
    pthread_mutex_init(&mailbox_lock, NULL);
    pthread_cond_init(&mailbox_cond, NULL);

    init_mailbox(&msg_mailbox);
}

void wait_post()
{
    pthread_mutex_lock(&mailbox_lock);
    pthread_cond_wait(&mailbox_cond, &mailbox_lock);
    pthread_mutex_unlock(&mailbox_lock);
}

bool deliver_mail(const Msg_pkg *msg)
{
    pthread_mutex_lock(&mailbox_lock);

    bool ret = push_mailbox(&msg_mailbox, msg);

    pthread_cond_signal(&mailbox_cond);
    pthread_mutex_unlock(&mailbox_lock);

    return ret;
}

bool fetch_mail(Msg_pkg *msg)
{
    traverse_mailbox(&msg_mailbox);
}

void show_mailbox()
{
    traverse_mailbox(&msg_mailbox);
}

bool is_mailbox_empty()
{
    return is_empty(&msg_mailbox);
}

void init_mailbox(Mailbox *mb)
{
    (void)memset(&(mb->msg_queue), 0xff, sizeof(mb->msg_queue));
    mb->front = 0;
    mb->rear = 0;
}

bool is_full(Mailbox *mb)
{
    if ((mb->rear + 1) % MAX_MSG_NUM == mb->front)
    {
        return true;
    }

    return false;
}

bool push_mailbox(Mailbox *mb, const Msg_pkg *msg)
{
    if (is_full(mb))
    {
        // CTC_TRACE_ERRO
        return false;
    }

    (void)memcpy(&(mb->msg_queue[mb->rear]), msg, sizeof(Msg_pkg));
    mb->rear = (mb->rear + 1) % MAX_MSG_NUM;

    return true;
}

//遍历队列
void traverse_mailbox(Mailbox *mb)
{
    int i = mb->front;
    
    CTC_TRACE_INFO(PRT, "|---------mailbox-----------|\n");
    while (i != mb->rear)
    {
        CTC_TRACE_INFO(PRT, "|--src[%4d]-event[0x%4x]-len[%4d]--|\n", mb->msg_queue[i].head.text.src_dir.pid, mb->msg_queue[i].head.text.event, mb->msg_queue[i].head.text.total_len);
        i = (i +1) % MAX_MSG_NUM;
    }

    CTC_TRACE_INFO(PRT, "|---------------------------|\n");

    return;
}

bool is_empty(Mailbox *mb)
{
    if (mb->front == mb->rear)
    {
        return true;
    }

    return false;
}

bool pop_mailbox(Mailbox *mb, Msg_pkg *msg)
{
    if (is_empty(mb))
    {
        // CTC_TRACE_ERRO
        return false;
    }

    (void)memcpy(msg, &(mb->msg_queue[mb->front]), sizeof(Msg_pkg));
    mb->front = (mb->front + 1) % MAX_MSG_NUM;

    return true;
}

int get_msg_num(const Mailbox *mb)
{
    int len = (mb->rear - mb->front + MAX_MSG_NUM) % MAX_MSG_NUM;

    return len;
}
