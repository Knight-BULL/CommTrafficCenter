#ifndef __ASSERT__H__
#define __ASSERT__H__

#include "com_def.h"
#include "trace_com.h"
#include <stdint.h>

#define ASSERT_SUCC_WITH(cond, ret, mode) do {                                          \
        if(cond)                                                                        \
        {                                                                               \
            uint16_t tmp_no = get_self_trace_no();                                      \ 
            TRACE_ERRO(tmp_no, mode, "Exec:[%s] FAILED, return %s!", #cond, #ret);      \
            return ret;                                                                 \
        }                                                                               \
    }while(0)

#define ASSERT_SUCC(cond, mode) do {                                                            \
        int ret_assert;                                                                         \
        if((ret_assert = cond))                                                                 \
        {                                                                                       \
            uint16_t tmp_no = get_self_trace_no();                                              \ 
            TRACE_ERRO(tmp_no, mode, "Exec:[%s] FAILED, return %s!", #cond, ret_assert);        \
            return ret_assert;                                                                  \
        }                                                                                       \
    }while(0)

#define ASSERT_SUCC_WITH_ACT(cond, act, mode) do {                                          \
        if(cond)                                                                            \
        {                                                                                   \
            uint16_t tmp_no = get_self_trace_no();                                          \ 
            TRACE_ERRO(tmp_no, mode, "Exec:[%s] FAILED, %s!", #cond, #act);                 \
            act;                                                                            \
        }                                                                                   \
    }while(0)

#define ASSERT_CONTINUE(cond, mode) do {                                                                \
        int ret_assert;                                                                                 \
        if(CONTINUE != (ret_assert = cond))                                                             \
        {                                                                                               \
            uint16_t tmp_no = get_self_trace_no();                                                      \ 
            TRACE_ERRO(tmp_no, mode, "Exec:[%s] NOT need CONTINUE, return %s!", #cond, ret_assert);     \
            return ret_assert;                                                                          \
        }                                                                                               \
    }while(0)

#define ASSERT_TRUE(cond, ret, mode) do {                                                   \
        if(!(cond))                                                                         \
        {                                                                                   \
            uint16_t tmp_no = get_self_trace_no();                                          \ 
            TRACE_ERRO(tmp_no, mode, "Cond:[%s] is false, return %s!", #cond, #ret);        \
            return ret;                                                                     \
        }                                                                                   \
    }while(0)

#define ASSERT_TRUE_WITH_ACT(cond, act, mode) do {                                          \
        if(!(cond))                                                                         \
        {                                                                                   \
            uint16_t tmp_no = get_self_trace_no();                                          \ 
            TRACE_ERRO(tmp_no, mode, "Cond:[%s] is false, %s!", #cond, #act);               \
            act;                                                                            \
        }                                                                                   \
    }while(0)

#define ASSERT_FALSE(cond, ret, mode) do {                                                  \
        if(cond)                                                                            \
        {                                                                                   \
            uint16_t tmp_no = get_self_trace_no();                                          \ 
            TRACE_ERRO(tmp_no, mode, "Cond:[%s] is true, return %s!", #cond, #ret);         \
            return ret;                                                                     \
        }                                                                                   \
    }while(0)


#define ASSERT_FALSE_WITH_ACT(cond, act, mode) do {                                         \
        if(cond)                                                                            \
        {                                                                                   \
            uint16_t tmp_no = get_self_trace_no();                                          \ 
            TRACE_ERRO(tmp_no, mode, "Cond:[%s] is true, %s!", #cond, #act);                \
            act;                                                                            \
        }                                                                                   \
    }while(0)

#define ASSERT_NOT_NULL(curr_ptr, ret, mode) do {                                           \
        if(NULL == curr_ptr)                                                                \
        {                                                                                   \
            uint16_t tmp_no = get_self_trace_no();                                          \       
            TRACE_ERRO(tmp_no, mode, "[%s] == NULL, return %s!", #curr_ptr, #ret);          \
            return ret;                                                                     \   
        }                                                                                   \
    }while(0)

#define ASSERT_NOT_NULL_WITH_ACT(curr_ptr, act, mode) do {                      \
        if(NULL == curr_ptr)                                                    \
        {                                                                       \ 
            uint16_t tmp_no = get_self_trace_no();                              \               
            TRACE_ERRO(tmp_no, mode, "[%s] == NULL, %s!", #curr_ptr, #act);     \                 
            act;                                                                \  
        }                                                                       \                                
    }while(0)

#endif
