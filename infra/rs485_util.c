#include <sys/ioctl.h>
#include <syslog.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
// #include "gpio_ioctl.h"
#include <stdint.h>
#include "rs485_util.h"
#include "com_def.h"
#include <time.h>
#include <pthread.h>

#define RS485_SEND_DATA_WAITTIME   (69)

#define GPIO_OFST       0x00000C00
#define GPIO_DAT        2

int ctc_gpio_fd = 0;
unsigned int *pGPIO = NULL;
int frs485_fd = 0;

typedef struct T_ack_syncer
{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    uint8_t is_busy;
}Ack_Syncer;

Ack_Syncer ack_syncer;

int set_serial_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    // struct termios newtio, oldtio;

    // if (tcgetattr(fd, &oldtio) != 0)
    // {
    //     perror("SetupSerial 1");
    //     return FAILED;
    // }
    

    return SUCCESS;
}


int iomon_get_frs485_fd(void)
{
    int frs485_fd;

    frs485_fd = open("/dev/ttyS8", O_RDWR|O_NOCTTY|O_NDELAY);
    if (frs485_fd < 0)
    {
        // TRACE_ERR
        return FAILED;
    }

    fcntl(frs485_fd, F_SETFL, 0);

    if (frs485_fd > 0)
    {
        set_serial_opt(frs485_fd, 115200, 8, 'N', 1);
    }
    else
    {
        TRACE_ERR(PRT, "the fserial device is busy");
    }


    return frs485_fd;
}

void create_ack_ayncer()
{
    pthread_mutex_init(&ack_syncer.lock, NULL);

    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&ack_syncer.cond, &attr);
    pthread_condattr_destroy(&attr);
}

void destroy_ack_ayncer()
{
    pthread_cond_destroy(&ack_syncer.cond);
    pthread_mutex_destroy(&ack_syncer.lock);
}

int init_gpio()
{
    ctc_gpio_fd = open("/dev/gpio_485_RE", O_RDWR);

    if (ctc_gpio_fd < 0)
    {
        // TRACE_ERR
        return FAILED;
    }

    if (ctc_gpio_fd > 0)
    {
        // ioctl(ctc_gpio_fd, SET_GPIO1_LOW);
    }

    return SUCCESS;
}

void set_rs485_tx_en(int state)
{
    if (state)
    {
        // ioctl(ctc_gpio_fd, SET_GPIO1_HIGH);
    }
    else
    {
        // ioctl(ctc_gpio_fd, SET_GPIO1_LOW);
    }
}

int open_rs485()
{
    if (FAILED == init_gpio())
    {
        // TRACE_ERR
        return FAILED;
    }

    create_ack_ayncer();

    frs485_fd = iomon_get_frs485_fd();

    if(-1 == frs485_fd)
    {
        // TRACE_ERR
        close(ctc_gpio_fd);
        destroy_ack_ayncer();
        return FAILED;
    }

    return SUCCESS;
}

void close_rs485()
{
    destroy_ack_ayncer();

    close(ctc_gpio_fd);
    close(frs485_fd);
}

int recv_from_rs485(void *_buff, const uint16_t _len)
{
    ASSERT_NOT_NULL(_buff, FAILED, PRT);

    return read(frs485_fd, _buff, _len);
}

int send_to_rs485(const void *_buff, const uint16_t _len)
{
    ASSERT_NOT_NULL(_buff, FAILED, PRT);

    usleep(10000);
    set_rs485_tx_en(1);
    usleep(100);
    write(frs485_fd, _buff, _len);
    usleep(_len * RS485_SEND_DATA_WAITTIME + 5000);
    set_rs485_tx_en(0);

    return SUCCESS;
}

void notify_recv()
{
    pthread_mutex_lock(&ack_syncer.lock);
    pthread_cond_signal(&ack_syncer.cond);
    pthread_mutex_unlock(&ack_syncer.lock);
}

struct timespec get_time_out_len()
{
    struct timespec abstime;
    struct timespec tp;
    uint32_t nsec = 0;
    uint32_t timeout_ms = 300;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    nsec = tp.tv_nsec + (timeout_ms % 1000) * 1000000;
    abstime.tv_nsec = nsec % 100000000;
    abstime.tv_sec = tp.tv_sec + nsec / 1000000000 + timeout_ms / 1000;

    return abstime;
}

int send_to_rs485_sync(const void* _data, const uint16_t _lenth)
{
    int reslt = 0;

    if (pthread_mutex_lock(&ack_syncer.lock) != 0)
    {
        return FAILED;
    }

    reslt = send_to_rs485(_data, _lenth);

    if (SUCCESS != reslt)
    {
        pthread_mutex_unlock(&ack_syncer.lock);

        // TRACE_ERR
        return FAILED;
    }

    struct timespec time_out = get_time_out_len();

    int ret_wait = pthread_cond_timedwait(&ack_syncer.cond, &ack_syncer.lock, &time_out);
    if (ret_wait == ETIMEDOUT)
    {
        // TRACE_ERR
        reslt = FAILED;
    }
    else
    {
        reslt = SUCCESS;
    }

    pthread_mutex_unlock(&ack_syncer.lock);

    return reslt;
}
