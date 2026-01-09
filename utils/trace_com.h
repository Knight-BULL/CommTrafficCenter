#ifndef __TRACE_COM__H__
#define __TRACE_COM__H__ 

#include <stdint.h>
#include <syslog.h>

#define LOG         ((uint8_t)0x01)
#define PRT         ((uint8_t)0x02)
#define TRC         ((uint8_t)(LOG | PRT))
#define NO_TRC      ((uint8_t)0x00)

#define IS_LOG(flag) (flag&LOG)?true:false
#define IS_PRT(flag) (flag&PRT)?true:false

#define LOG_INFO(format, ...)  syslog(LOG_LOCAL4 | LOG_DEBUG, "-INFO- [%s: %s() L:%d]: "format"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERRO(format, ...)  syslog(LOG_LOCAL4 | LOG_DEBUG, "-ERRO- [%s: %s() L:%d]: "format"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define PRT_INFO(format, ...)  printf("-INFO- [%s: %s() L:%d]: "format"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define PRT_ERRO(format, ...)  printf("-ERRO- [%s: %s() L:%d]: "format"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

static uint8_t ctc_trc_state = NO_TRC;

typedef enum E_Trace_list
{
    INVALID_TRACE = 0,
    CTC_TRACE = 0x01,

}Trace_list;

static uint8_t get_trace_state(const uint16_t trc_no)
{
    switch (trc_no)
    {
        case CTC_TRACE:
            return ctc_trc_state;
        
        default:
            break;
    }

    return 0;
}

#define TRACE_INFO(trace_no, mode, format, ...) do {             \
        uint8_t tmp_state = get_trace_state(trace_no);           \
        if (IS_PRT(tmp_state & mode))                            \
            PRT_INFO(format, ##__VA_ARGS__);                     \
        if (IS_LOG(tmp_state & mode))                            \
            LOG_INFO(format, ##__VA_ARGS__);                     \
        }while(0)

#define TRACE_ERRO(trace_no, mode, format, ...) do {             \
        uint8_t tmp_state = get_trace_state(trace_no);           \
        if (IS_PRT(tmp_state & mode))                            \
            PRT_ERRO(format, ##__VA_ARGS__);                     \
        if (IS_LOG(tmp_state & mode))                            \
            LOG_ERRO(format, ##__VA_ARGS__);                     \
        }while(0)

#endif
