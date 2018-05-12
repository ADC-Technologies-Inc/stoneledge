/*
 * rhu.c
 *
 *  Created on: Feb 25, 2018
 *      Author: Zack Lyzen
 */

#define DEBUG_RHU

#include "../prog/rhu.h"


#ifdef DEBUG_RHU
#include "ntd_debug.h"
#endif

/*structs*/
struct rhu_state__{
    uint16_t    en;
    uint16_t    has_power;
    uint16_t    duty;
};

//===========================================================================
// private variables
//===========================================================================

#ifdef LOW_DUTY_MODE
const static uint16_t DUTY_TABLE[4] = {0, 50, 75, 100};
#else
const static uint16_t DUTY_TABLE[4] = {0, 500, 75, 1000};
#endif

static struct rhu_state__ rhu_state[RHU_COUNT] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};

/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Running
 *
 -----------------------------------------------------------------------------------------------------------*/

/*Turn on the 48v part of the board (RHUs)*/
void RHU_Enable48V(void){
#ifdef DEBUG_RHU
    printf("RHU_Enable48V():: Setting GPIO40 ON\n" );
#endif

	GpioDataRegs.GPBSET.bit.GPIO40 = 0x01;
}

/*Turn on the 48v part of the board (RHUs)*/
void RHU_Disable48V(void)
{
#ifdef DEBUG_RHU
    printf("RHU_Disable48V():: Clearing GPIO40\n" );
#endif

	GpioDataRegs.GPBCLEAR.bit.GPIO40 = 0x01;
}

/*Verify that the fust is on*/
Uint16 RHU_Get48VFuse(void){
    uint16_t state = GpioDataRegs.GPADAT.bit.GPIO25;

    #ifdef DEBUG_RHU
    printf("RHU_Get48VFuse():: Returning 48V Fuse (GPIO40) state as %d\n", state );
    #endif

	return state;
}

/*This function is called when RHU TCOs should be read instantaneously, will be called from within ePWM1 ISR- quick as poss.*/
void RHU_PWMCallback( void ){
    uint16_t toggle = 0;
    uint16_t gpio_set;

    /*
     * Poll RHU power status sets and save data for next control loop read, we do one at a time to minimize i2c traffic while the duty cycle is running
     *
     * Since we're in a high-priority ISR we do not do the work of verifying here, we do that at the next control loop iteration.
     *
     * */
    if (toggle){
        //Pull GPIO 500 data set
        gpio_set = ExtGpioGetSet(5);

        /*/Update local status
         *
         * 0x10 - 504              = RHU1 = CPU1
         * 0x20 - 505              = RHU2 = CPU2
         * 0x40 - 506              = RHU3 = RAM
         * 0x01 - 500 (was 507)    = RHU4 = MISC
         *
         */

        rhu_state[CPU1].has_power = gpio_set & 0x10;
        rhu_state[CPU2].has_power = gpio_set & 0x20;
        rhu_state[RAM].has_power = gpio_set & 0x40;
        rhu_state[MISC].has_power = gpio_set & 0x01;
    }else{
        //Pull GPIO 600 data set
        gpio_set = ExtGpioGetSet(6);

        /*/Update local status
         *
         * 0x01 - 600              = RHU5 = M.2
         * 0x02 - 601              = RHU6 = DIMM
         * 0x04 - 602              = RHU7 = MEZZ
         * 0x08 - 603              = RHU8 = SFF
         *
         */

        rhu_state[M_2_GRP].has_power = gpio_set & 0x01;
        rhu_state[DIMM_GRP].has_power = gpio_set & 0x02;
        rhu_state[MEZZ].has_power = gpio_set & 0x04;
        rhu_state[SFF_GRP].has_power = gpio_set & 0x08;
    }
}

void RHU_VerifyRHU(void)
{
#ifdef DEBUG_RHU
    printf("RHU_VerifyRHU():: Go through TCO status and check for errors\n" );
#endif

	int i = 0, x;

	while(i < RHU_COUNT){
	    //nb. we turn on has_power when we enable to ensure we don't get caught out by ControlLoop calling this before RHU_PWMCallback() has had a chance to be called
	    if (rhu_state[i].duty && rhu_state[i].en && !rhu_state[i].has_power ) {
            #ifdef DEBUG_RHU
            printf("RHU_VerifyRHU():: Flagging error on TCO %d\n", i );
            #endif

            /*RHU HARDSTOP*/
            for (x = 0; x < RHU_COUNT; x++){
                LED_Set(x);
                LED_Set(LED_TCO);
            }

            /*SYSTEM should now enter HALT mode until reset*/
            CTL_HardSTOP(FAIL_TCO_PTC);
	    }
	}
}

void RHU_EnableRHU(uint16_t rhu_)
{
    #ifdef DEBUG_RHU
    printf("RHU_EnableRHU():: Enabling RHU %d for duty %d\n", rhu_, rhu_state[rhu_].duty );
    #endif

    if (!rhu_state[rhu_].duty){
        #ifdef DEBUG_RHU
        printf("RHU_EnableRHU():: RHU not enabled, nothing to do\n" );
        #endif

        return;
    }

    rhu_state[rhu_].en = 1;
    rhu_state[rhu_].has_power = 1;
    PWM_SetDuty( rhu_, rhu_state[rhu_].duty );
}

void RHU_DisableRHU(uint16_t rhu_)
{
    #ifdef DEBUG_RHU
    printf("RHU_DisableRHU():: Disabling RHU %d\n", rhu_ );
    #endif

    rhu_state[rhu_].en = 0;
    PWM_SetDuty( rhu_, 0 );
}

void RHU_EStopRHU(void)
{
    #ifdef DEBUG_RHU
        printf("RHU_EStopRHU():: Bringing down 48V system and setting RHUs duties to 0)\n");
    #endif

	int i = 0;

    RHU_Disable48V();

    //all duty cycles to 0
	for (i = 0; i < RHU_COUNT; i++){
	    RHU_DisableRHU(i);
	}
}

#define READDUTY(a_,b_) ( ExtGpioRead(a_) ? 0x02 : 0 ) +  ( ExtGpioRead(b_) ? 0x01 : 0 )
void RHU_Init(void)
{
	uint16_t temp_duty;

	// RHU 1
	temp_duty = READDUTY(300, 301);
	if(USE_RHU_1)
	    rhu_state[0].duty = DUTY_TABLE[temp_duty];

	// RHU 2
	temp_duty = READDUTY(302, 303);
	if(USE_RHU_2)
        rhu_state[1].duty = DUTY_TABLE[temp_duty];

	// RHU 3
	temp_duty = READDUTY(304, 305);
	if(USE_RHU_3)
        rhu_state[2].duty = DUTY_TABLE[temp_duty];

	// RHU 4
	temp_duty = READDUTY(306, 307);
	if(USE_RHU_4)
        rhu_state[3].duty = DUTY_TABLE[temp_duty];

	// RHU 5
	temp_duty = READDUTY(400, 401);
	if(USE_RHU_5)
        rhu_state[4].duty = DUTY_TABLE[temp_duty];

	// RHU 6
	temp_duty = READDUTY(402, 403);
	if(USE_RHU_6)
        rhu_state[5].duty = DUTY_TABLE[temp_duty];

	//RHU 7
	temp_duty = READDUTY(404, 405);
	if(USE_RHU_7)
        rhu_state[6].duty = DUTY_TABLE[temp_duty];

	// RHU 8
	temp_duty = READDUTY(406, 407);
	if(USE_RHU_8)
        rhu_state[7].duty = DUTY_TABLE[temp_duty];
}


