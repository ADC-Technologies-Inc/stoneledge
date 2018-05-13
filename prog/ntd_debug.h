#ifndef DEBUG
#define DEBUG
#endif

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

#ifdef DEBUG
#define ASSERT_(x_) if ( (!x_) ) printf("ASSERT FAILED "__FILE__" line: %d\n", __LINE__ );
#define ASSERT( x_ ) ASSERT_( (x_) )

#elif
#define ASSERT( x_ )
#endif

