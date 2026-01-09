#ifndef __CTC_TRACE__H__
#define __CTC_TRACE__H__

#include "com_def.h"
#include "trace_com.h"
#include <stdint.h>

uint16_t get_self_trace_no();

#define CTC_TRACE_INFO(trc_mode, format, ...) TRACE_INFO(CTC_TRACE, trc_mode, format, ##__VA_ARGS__)
#define CTC_TRACE_ERRO(trc_mode, format, ...) TRACE_ERRO(CTC_TRACE, trc_mode, format, ##__VA_ARGS__)

#endif
