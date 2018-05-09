#ifndef DEBUG
#define DEBUG
#endif

#ifdef DEBUG

#include "stdio.h"

#define PRINTF_BINSTR8 "%d%d%d%d%d%d%d%d"
#define PRINTF_BINSTR8_ARGS(in_)\
        (in_ & 0x80)>>7,\
        (in_ & 0x40)>>6,\
        (in_ & 0x20)>>5,\
        (in_ & 0x10)>>4,\
        (in_ & 0x08)>>3,\
        (in_ & 0x04)>>2,\
        (in_ & 0x02)>>1,\
        in_ & 0x01\

#endif
