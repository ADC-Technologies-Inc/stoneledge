#include "prog_conf.h"

#ifdef DEBUG
#include "stdio.h"
#endif

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

#define PRINTF_BINSTR16 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
#define PRINTF_BINSTR16_ARGS(in_)\
        (in_ & 0x8000)>>15,\
        (in_ & 0x4000)>>14,\
        (in_ & 0x2000)>>13,\
        (in_ & 0x1000)>>12,\
        (in_ & 0x800)>>11,\
        (in_ & 0x400)>>10,\
        (in_ & 0x200)>>9,\
        (in_ & 0x100)>>8,\
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
#else
#define ASSERT( x_ )
#endif

