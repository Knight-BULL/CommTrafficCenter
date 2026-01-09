#ifndef _COM_DEF__H__
#define _COM_DEF__H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

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
 