#ifndef _COM_DEF__H__
#define _COM_DEF__H__

#include <stdint.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define LOG         ((uint8_t)0x01)
#define PRT         ((uint8_t)0x02)
#define TRC         ((uint8_t)(LOG | PRT))
#define NO_TRC      ((uint8_t)0x00)

#define IS_LOG(flag) (flag&LOG)?true:false
#define IS_PRT(flag) (flag&PRT)?true:false

#define LOG_INFO(format, ...)  syslog(LOG_LOCAL4 | LOG_DEBUG, "-INFOR- [%s: %s() L:%d]: "format"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERRO(format, ...)  syslog(LOG_LOCAL4 | LOG_DEBUG, "-ERROR- [%s: %s() L:%d]: "format"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define PRT_INFO(format, ...)  printf("-INFOR- [%s: %s() L:%d]: "format"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define PRT_ERRO(format, ...)  printf("-ERROR- [%s: %s() L:%d]: "format"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define TRACE_INFO(trace, format, ...) do {             \
        if(IS_PRT(trace))                               \
            PRT_INFO(""format"", ##__VA_ARGS__);        \
        if(IS_LOG(trace))                               \
            LOG_INFO(""format"", ##__VA_ARGS__);        \
        }while(0)

#define TRACE_ERRO(trace, format, ...) do {             \
        if(IS_PRT(trace))                               \
            PRT_ERRO(""format"", ##__VA_ARGS__);        \
        if(IS_LOG(trace))                               \
            LOG_ERRO(""format"", ##__VA_ARGS__);        \
        }while(0)

#define ASSERT_SUCC_WITH(cond, ret, trace) do {                             \
        if(cond)                                                            \
        {                                                                   \
            if(IS_PRT(trace))                                               \
                PRT_ERRO("Exec:[%s] FAILED, return %s!", #cond, #ret);      \
            if(IS_LOG(trace))                                               \
                LOG_ERRO("Exec:[%s] FAILED, return %s!", #cond, #ret);      \
            return ret;                                                     \
        }                                                                   \
    }while(0)

#define ASSERT_SUCC(cond, trace) do {                                           \
        int ret_assert;                                                         \
        if((ret_assert = cond))                                                 \
        {                                                                       \
            if(IS_PRT(trace))                                                   \
                PRT_ERRO("Exec:[%s] FAILED, return %s!", #cond, ret_assert);    \
            if(IS_LOG(trace))                                                   \
                LOG_ERRO("Exec:[%s] FAILED, return %s!", #cond, ret_assert);    \
            return ret_assert;                                                  \
        }                                                                       \
    }while(0)

#define ASSERT_SUCC_WITH_ACT(cond, act, trace) do {                             \
        if(cond)                                                                \
        {                                                                       \
            if(IS_PRT(trace))                                                   \
                PRT_ERRO("Exec:[%s] FAILED, %s!", #cond, #act);                 \
            if(IS_LOG(trace))                                                   \
                LOG_ERRO("Exec:[%s] FAILED, %s!", #cond, #act);                 \
            act;                                                                \
        }                                                                       \
    }while(0)

#define ASSERT_CONTINUE(cond, trace) do {                                                   \
        int ret_assert;                                                                     \
        if(CONTINUE != (ret_assert = cond))                                                 \
        {                                                                                   \
            if(IS_PRT(trace))                                                               \
                PRT_ERRO("Exec:[%s] NOT need CONTINUE, return %s!", #cond, ret_assert);     \
            if(IS_LOG(trace))                                                               \
                LOG_ERRO("Exec:[%s] NOT need CONTINUE, return %s!", #cond, ret_assert);     \
            return ret_assert;                                                              \
        }                                                                                   \
    }while(0)

#define ASSERT_TRUE(cond, ret, trace) do {                                      \
        if(!(cond))                                                             \
        {                                                                       \
            if(IS_PRT(trace))                                                   \
                PRT_ERRO("Cond:[%s] is false, return %s!", #cond, #ret);        \
            if(IS_LOG(trace))                                                   \
                LOG_ERRO("Cond:[%s] is false, return %s!", #cond, #ret);        \
            return ret;                                                         \
        }                                                                       \
    }while(0)

#define ASSERT_TRUE_WITH_ACT(cond, act, trace) do {                             \
        if(!(cond))                                                             \
        {                                                                       \
            if(IS_PRT(trace))                                                   \
                PRT_ERRO("Cond:[%s] is false, %s!", #cond, #act);               \
            if(IS_LOG(trace))                                                   \
                LOG_ERRO("Cond:[%s] is false, %s!", #cond, #act);               \
            act;                                                                \
        }                                                                       \
    }while(0)

#define ASSERT_FALSE(cond, ret, trace) do {                                     \
        if(cond)                                                                \
        {                                                                       \
            if(IS_PRT(trace))                                                   \
                PRT_ERRO("Cond:[%s] is true, return %s!", #cond, #ret);         \
            if(IS_LOG(trace))                                                   \
                LOG_ERRO("Cond:[%s] is true, return %s!", #cond, #ret);         \
            return ret;                                                         \
        }                                                                       \
    }while(0)


#define ASSERT_FALSE_WITH_ACT(cond, act, trace) do {                            \
        if(cond)                                                                \
        {                                                                       \
            if(IS_PRT(trace))                                                   \
                PRT_ERRO("Cond:[%s] is true, %s!", #cond, #act);                \
            if(IS_LOG(trace))                                                   \
                LOG_ERRO("Cond:[%s] is true, %s!", #cond, #act);                \
            act;                                                                \
        }                                                                       \
    }while(0)

#define ASSERT_NOT_NULL(curr_ptr, ret, trace) do {                              \
        if(NULL == curr_ptr)                                                    \
        {                                                                       \
            if(IS_PRT(trace))                                                   \
                PRT_ERRO("[%s] == NULL, return %s!", #curr_ptr, #ret);          \
            if(IS_LOG(trace))                                                   \
                LOG_ERRO("[%s] == NULL, return %s!", #curr_ptr, #ret);          \
            return ret;                                                         \
        }                                                                       \
    }while(0)

#define ASSERT_NOT_NULL_WITH_ACT(curr_ptr, act, trace) do {                     \
        if(NULL == curr_ptr)                                                    \
        {                                                                       \
            if(IS_PRT(trace))                                                   \
                PRT_ERRO("[%s] == NULL, %s!", #curr_ptr, #act);                 \
            if(IS_LOG(trace))                                                   \
                LOG_ERRO("[%s] == NULL, %s!", #curr_ptr, #act);                 \
            act;                                                                \
        }                                                                       \
    }while(0)

#define container_of(ptr, type, member) ({                          \      
    const typeof( ((type *)0)->member) *__mptr = (ptr);             \
    (type *)( (char *)__mptr - OFFSET(type, member) );              \
})

#define OFFSET(type, member) ((size_t)&((type *)0)->member)

#define MAX_MSG_NUM 64

#define INVALID_UINT8     ((uint8_t)0xFF)
#define INVALID_UINT16    ((uint8_t)0xFFFF)
#define INVALID_UINT32    ((uint8_t)0xFFFFFFFF)

#define SUCCESS           ((uint8_t)0)
#define FAILED            ((uint8_t)-1)
#define CONTINUE          ((uint8_t)1)

#ifndef NULL
#define NULL (void*)0
#endif

#endif
 