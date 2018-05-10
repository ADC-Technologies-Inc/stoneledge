#include "duty_sw.h"

//#define DEBUG_DUTY_SW 1

#ifdef DEBUG_DUTY_SW
#include "ntd_debug.h"
#endif

/*
 * DIP SWITCHES SW1 and SW1 are configured as follows
 *
 * SW1 (300-307)
 * 1+2 = CPU1
 * 3+4 = CPU2
 * 5+6 = MISC
 * 7+8 = RAM
 *
 * SW2 (400-407)
 *
 * 1+2 = DIMM_GRP
 * 3+4 = M_2_GRP
 * 5+6 = SFF_GRP
 * 7+8 = MEZZ
 */

static uint16_t dutySettings[8];

//Prv Decl
void ReadDutySW( uint16_t a_, uint16_t b_, uint16_t rhu_ );

//Converts duty switch inputs to appropriate duty cycle
void ReadDutySW( uint16_t p1_, uint16_t p2_, uint16_t rhu_ ){
    uint16_t a, b;

    a = ExtGpioRead(p1_);
    b = ExtGpioRead(p2_);
    dutySettings[rhu_-1] = ( (a) ? ((b) ? 100 : 50 ) : ((b) ? 75 : 0));\

#ifdef DEBUG_DUTY_SW
    printf( "Duty for RHU %d set to %d - a=%d b=%d\n", rhu_, dutySettings[rhu_-1],a,b );
#endif
}

void InitDutySW(void)
{
    //Read duty switches into appropriate locations.
    ReadDutySW( 300, 301, CPU1 );
    ReadDutySW( 302, 303, CPU2 );
    ReadDutySW( 304, 305, MISC );
    ReadDutySW( 306, 307, RAM );

    ReadDutySW( 400, 401, DIMM_GRP );
    ReadDutySW( 402, 403, M_2_GRP );
    ReadDutySW( 404, 405, SFF_GRP );
    ReadDutySW( 406, 407, MEZZ );
}

uint16_t GetDuty(uint16_t rhu_)
{
    return dutySettings[rhu_-1];
}
