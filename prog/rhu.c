/*
 * rhu.c
 *
 *  Created on: Feb 25, 2018
 *      Author: Zack Lyzen
 */

#include "../prog/rhu.h"
#include "ntd_debug.h"

/*structs*/
struct rhu_state__{
    uint16_t    en;
    uint16_t    has_power;
    uint16_t    duty;
    uint16_t    ramp;
    uint16_t    ramp_inc;
    uint16_t    watchdog;
    uint32_t    wd_last_overtemp;
    uint32_t    wd_lastramp;
};

//defines to convert tick to percentages
#define PERCCONV_DIV    100
#define PERCCONV_MUL    100

//===========================================================================
// private variables
//===========================================================================

#ifdef LOW_DUTY_MODE    //5%, 7.5%, 10%
const static uint16_t DUTY_TABLE[4] = {0, 500, 750, 1000};
const static uint16_t RAMP_TABLE[4] = {0, 100, 150, 200 };
#else                   //50%, 75%, 100%
const static uint16_t DUTY_TABLE[4] = {0, 5000, 7500, 10000};
const static uint16_t RAMP_TABLE[4] = {0, 1000, 1500, 2000 };
#endif

static struct rhu_state__ rhu_state[RHU_COUNT] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};

//Counter, increment for each RHU added to watchdog, decrement when removed
static uint16_t wd_active = 0;

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

/*
 * Turn on the 48v part of the board (RHUs)
 *
 * WARNING: DO NOT DISABLE 48V WHILE DUTY-CYCLES ARE ACTIVE, REDUCE DUTY-CYCLES TO 0 FIRST TO PREVENT DAMAGE TO RELAYS
 *
 * */

void RHU_Disable48V(void)
{
#ifdef DEBUG_RHU
    printf("RHU_Disable48V():: Clearing GPIO40\n" );
#endif

	GpioDataRegs.GPBCLEAR.bit.GPIO40 = 0x01;
}

/*Verify that the fuse is on*/
Uint16 RHU_Get48VFuse(void){
    uint16_t state = !GpioDataRegs.GPADAT.bit.GPIO25;   //inverted

    #ifdef DEBUG_RHU
    printf("RHU_Get48VFuse():: Returning 48V Fuse (GPIO40) state as %d\n", state );
    #endif

	return state;
}

/*This function is called when RHU TCOs should be read instantaneously, will be called from within ePWM1 ISR- quick as poss.*/
void RHU_PWMCallback( void ){
    uint16_t toggle = 1;
    uint16_t gpio_set;


    /*TODO*/
    //WE MAY MAKE THIS REQUIRE 3 CONSECUTIVE FAILS TO CONSTITUTE A FAIL

    /*
     * Poll RHU power status sets and save data for next control loop read, we do one at a time to minimize i2c traffic while the duty cycle is running
     *
     * Since we're in a high-priority ISR we do not do the work of verifying here, we do that at the next control loop iteration.
     *
     * */
    if (toggle){
        //Pull GPIO 500 data set
        gpio_set = ExtGpioGetSet(5);
        #ifdef DEBUG_RHU
        printf("RHU_PWMCallback():: 500 GPIOSET="PRINTF_BINSTR8"\n", PRINTF_BINSTR8_ARGS(gpio_set) );
        #endif

        /*/Update local status
         *
         * 0x10 - 504              = RHU1 = CPU1
         * 0x20 - 505              = RHU2 = CPU2
         * 0x40 - 506              = RHU3 = RAM
         * 0x01 - 500 (was 507)    = RHU4 = MISC
         *
         */

        rhu_state[CPU1].has_power = !(gpio_set & 0x10);
        rhu_state[CPU2].has_power = !(gpio_set & 0x20);
        rhu_state[RAM].has_power = !(gpio_set & 0x40);
        rhu_state[MISC].has_power = !(gpio_set & 0x01);
        toggle = 0;
    }else{
        //Pull GPIO 600 data set
        gpio_set = ExtGpioGetSet(6);
        #ifdef DEBUG_RHU
        printf("RHU_PWMCallback():: 600 GPIOSET="PRINTF_BINSTR8"\n", PRINTF_BINSTR8_ARGS(gpio_set) );
        #endif

        /*/Update local status
         *
         * 0x01 - 600              = RHU5 = M.2
         * 0x02 - 601              = RHU6 = DIMM
         * 0x04 - 602              = RHU7 = MEZZ
         * 0x08 - 603              = RHU8 = SFF
         *
         */

        rhu_state[M_2_GRP].has_power = !(gpio_set & 0x01);
        rhu_state[DIMM_GRP].has_power = !(gpio_set & 0x02);
        rhu_state[MEZZ].has_power = !(gpio_set & 0x04);
        rhu_state[SFF_GRP].has_power = !(gpio_set & 0x08);
        toggle = 1;
    }
}

void RHU_VerifyRHU(uint16_t fail_msg_)
{
    uint16_t flag_hardstop = 0;

    #ifdef DEBUG_RHU
    printf("RHU_VerifyRHU():: Go through TCO status and check for errors\n" );
    #endif

	int i = 0;

	for( i=0; i < RHU_COUNT; i++ ){
	    //nb. we turn on has_power when we enable to ensure we don't get caught out by ControlLoop calling this before RHU_PWMCallback() has had a chance to be called
	    ASSERT( (rhu_state[i].en) ? rhu_state[i].duty : 1 );

	    if (rhu_state[i].en && !rhu_state[i].has_power ) {
            #ifdef DEBUG_RHU
            printf("RHU_VerifyRHU():: Flagging error on TCO %d\n", i );
            #endif

            //HardSTOP, turn on LED and continue around in case there are multiple errors
            LED_Set(i);
            flag_hardstop = 1;
	    }
	}

	if (flag_hardstop){
        #ifdef DEBUG_RHU
        printf("RHU_VerifyRHU():: Hardstop flagged, calling CTL_HardSTOP()\n", i );
        #endif

        LED_Set(LED_TCO);
	    CTL_HardSTOP(fail_msg_);
	}
}

uint16_t RHU_GetSetDutyPerc(uint16_t rhu_){

    //Convert into percentage and return
    return rhu_state[rhu_].duty / PERCCONV_DIV;
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
    rhu_state[rhu_].ramp = rhu_state[rhu_].duty;
    PWM_SetDuty( rhu_, rhu_state[rhu_].duty );
}

/*Repeatedly called, slowly ramps up the RHU to full power
 *
 * RHU_FULLPOWER               //RHU is complete, don't call again (please k thx)
 * RHU_ABORT                   //Error of some form, presently unused
 * RHU_DISABLED_BY_SWITCH      //Duty switch is off
 * RHU_DISABLED                //Disabled in software
 *
 * otherwise return is percentage of ramp complete
 *
 */
int RHU_EnableRHU_RAMP(uint16_t rhu_){
    ASSERT( rhu_ < RHU_COUNT );

    //Check if RHU is software disabled
    #if USE_RHU_1 == 0
    if (rhu_ == 0) return RHU_DISABLED;
    #endif
    #if USE_RHU_2 == 0
    if (rhu_ == 1) return RHU_DISABLED;
    #endif
    #if USE_RHU_3 == 0
    if (rhu_ == 2) return RHU_DISABLED;
    #endif
    #if USE_RHU_4 == 0
    if (rhu_ == 3) return RHU_DISABLED;
    #endif
    #if USE_RHU_5 == 0
    if (rhu_ == 4) return RHU_DISABLED;
    #endif
    #if USE_RHU_6 == 0
    if (rhu_ == 5) return RHU_DISABLED;
    #endif
    #if USE_RHU_7 == 0
    if (rhu_ == 6) return RHU_DISABLED;
    #endif
    #if USE_RHU_8 == 0
    if (rhu_ == 7) return RHU_DISABLED;
    #endif

    //Check if RHU is switch disabled
    if ( rhu_state[rhu_].duty == 0 ) return RHU_DISABLED_BY_SWITCH;

    //Check that if the RHU is showing enabled that the ramp is correct
    ASSERT( (rhu_state[rhu_].en) ? rhu_state[rhu_].ramp : !rhu_state[rhu_].ramp );

    //Check if we've been called while already at full power or if ramp > duty, shouldn't happen
    ASSERT(rhu_state[rhu_].ramp <= rhu_state[rhu_].duty);

    if (!rhu_state[rhu_].en){
        rhu_state[rhu_].en = 1;
        rhu_state[rhu_].has_power = 1;
    }

    //Set next ramp
    rhu_state[rhu_].ramp += rhu_state[rhu_].ramp_inc;
    if (rhu_state[rhu_].ramp > rhu_state[rhu_].duty) rhu_state[rhu_].ramp = rhu_state[rhu_].duty;

    //Set duty
    PWM_SetDuty( rhu_, rhu_state[rhu_].ramp );

    //Are we now at full power?
    if (rhu_state[rhu_].ramp == rhu_state[rhu_].duty) return RHU_FULLPOWER;


#ifdef SHOW_DUTY
    //Show duty shows the duty-cycle setting when ramping
    return rhu_state[rhu_].ramp / PERCCONV_DIV;
#else
    //Show instead the % towards the configured power
    return (rhu_state[rhu_].ramp / rhu_state[rhu_].ramp_inc) * 20;
#endif
}

void RHU_DisableRHU(uint16_t rhu_)
{
    #ifdef DEBUG_RHU
    printf("RHU_DisableRHU():: Disabling RHU %d\n", rhu_ );
    #endif

    rhu_state[rhu_].en = 0;
    rhu_state[rhu_].ramp = 0;
    rhu_state[rhu_].has_power = 0;
    PWM_SetDuty( rhu_, 0 );
}

void RHU_EStopRHU(void)
{
    #ifdef DEBUG_RHU
        printf("RHU_EStopRHU():: Bringing down 48V system and setting RHUs duties to 0)\n");
    #endif

	int i = 0;

    //all duty cycles to 0
	for (i = 0; i < RHU_COUNT; i++){
	    RHU_DisableRHU(i);
	}

	//Hold for a second and _then_ bring down the 48v relays
	DELAY_US(1000000);
    RHU_Disable48V();
}

#define READDUTY(a_,b_) ( ExtGpioRead(a_) ? 0x02 : 0 ) +  ( ExtGpioRead(b_) ? 0x01 : 0 )

#ifdef DEBUG_RHU
#define SETDUTY(a_,b_, c_) temp_duty = READDUTY(a_, b_);\
        rhu_state[c_].duty = DUTY_TABLE[temp_duty];\
        rhu_state[c_].ramp_inc = RAMP_TABLE[temp_duty];\
        printf("RHU_Init():: RHU %d duty switches read %d\n",c_,temp_duty);
#else
#define SETDUTY(a_,b_, c_) temp_duty = READDUTY(a_, b_);\
        rhu_state[c_].duty = DUTY_TABLE[temp_duty];\
        rhu_state[c_].ramp_inc = RAMP_TABLE[temp_duty];
#endif

void RHU_Init(void)
{
	uint16_t temp_duty;

	// RHU 1 - CPU1
    #if USE_RHU_1
	SETDUTY(300, 301, 0)
    #endif

	// RHU 2 - CPU2
    #if USE_RHU_2
	SETDUTY(302, 303, 1)
    #endif

	// RHU 3 - MISC IC
    #if USE_RHU_3
    SETDUTY(304, 305, 2)
    #endif

	// RHU 4 - RAM, we have a hardware problem with the on-board RAM, hard setting duty to prevent overheat (engineering fail, Niall fail not Thermal Rail fail)
    #if USE_RHU_4
    rhu_state[3].duty = 4000;
    rhu_state[3].ramp_inc = 800;
    //SETDUTY(306, 307, 3)
    #endif

    // RHU 5 - DIMMs
    #if USE_RHU_5
    SETDUTY(400, 401, 4)
    #endif

    // RHU 6 - M.2
    #if USE_RHU_6
    SETDUTY(402, 403, 5)
    #endif

    // RHU 7 - SFF
    #if USE_RHU_7
    SETDUTY(404, 405, 6)
    #endif

    // RHU 8 - Mezz
    #if USE_RHU_8
    SETDUTY(406, 407, 7)
    #endif
}

void RHU_Watchdog_FAIL(uint16_t rhu_){
    //Add to watchdog.
    ASSERT( rhu_ < RHU_COUNT );

    //Update time.
    rhu_state[rhu_].wd_last_overtemp = time_ms;

    //check if rhu is already under the watchdogs management, if it is get out
    if ( rhu_state[rhu_].watchdog ) return;

    //not in watchdog
    #ifdef DEBUG_RHU
    printf("RHU_Watchdog_FAIL():: RHU %d taken offline by watchdog\n", rhu_ );
    #endif


    //Since we're being called related to a thermal issue if there is no power applied to this RHU it's not this RHU that's the problem- it's pretty severe though so hardstop
    if ( !rhu_state[rhu_].en ){

         //not enabled, the problem isn't going to be solved by turning off this RHU, a neighbor might be too hot?
        #ifdef DEBUG_RHU
        printf("RHU_Watchdog_FAIL():: Overheating RHU not ONLINE, calling CTL_HardSTOP()\n", rhu_ );
        #endif

        LED_Set(rhu_);
        CTL_HardSTOP(FAIL_OVERHEAT_RHU_NOT_ON);
    }

    //Shutdown the RHU and mark it for watchdog service
    RHU_DisableRHU(rhu_);
    ASSERT(rhu_state[rhu_].ramp == 0 && rhu_state[rhu_].en == 0);

    rhu_state[rhu_].watchdog = 1;
    wd_active++;
    LED_Set(rhu_);

    ASSERT( wd_active > 0 && wd_active < RHU_COUNT );

    //was this the first watchdog?
    if (wd_active == 1){
        LED_Set(LED_ONGOING);
        LcdPostStatic(OVERHEAT);
    }
}


/*GOT HERE*/
void RHU_Watchdog_Service(){
    int ret;
    uint16_t i, temp, overheat = 0;
    uint32_t ms_since_last;

#ifdef DEBUG_RHU
    //DEBUG, verify nothing is active while the counter is 0
    if (!wd_active){
        for (i = 0; i < RHU_COUNT; i++){
            ASSERT(rhu_state[i].watchdog == 0);
        }
        return;
    }
#else
    if (!wd_active) return; //nothing to do
#endif

    for (i=0; i< RHU_COUNT; i++){
        if (rhu_state[i].watchdog ){


            if ( rhu_state[i].en ){
                //RHU is being ramped up again, check if the temperature has exceeded safety limits
                if ( TEMPS_GetFlag(i) ){
                    //turn it off again and reset event timer
                    rhu_state[i].wd_last_overtemp = time_ms;
                    RHU_DisableRHU(i);
                    continue;
                }

                //Temps are still ok, has enough time passed since the last ramp?
                ms_since_last = time_ms - rhu_state[i].wd_lastramp;
                if (ms_since_last < ((i < 2) ? RAMP_DELAY_CPU : RAMP_DELAY) ) continue;     //nothing to do for this RHU

                //Continue out to ramp up...
            }else{
                //Check if enough time has passed since overheat condition to turn on the RHU again
                ms_since_last = time_ms - rhu_state[i].wd_last_overtemp;
                if (ms_since_last < RHU_WATCHDOG_DELAY ) continue;    //nothing to do for this RHU on this iteration

                //Continue out to ramp up...
            }

            //Ramp up
            ret = RHU_EnableRHU_RAMP(i);
            switch(ret){
            case RHU_FULLPOWER:{
                    //Ramp complete, remove from watchdog and treat it like a normal RHU again
                    rhu_state[i].wd_lastramp = 0;
                    rhu_state[i].wd_last_overtemp = 0;
                    rhu_state[i].watchdog = 0;
                    wd_active--;
                    LED_Clear(i);
                    break;
                }
            case RHU_ABORT:{
                    //Abort. Danger Will Robinson, Abort Abort.
                    #ifdef DEBUG_RHU
                    printf("RHU_Watchdog_FAIL():: RHU_EnableRHU_RAMP() returned RHU_ABORT, calling CTL_HardSTOP()\n" );
                    #endif

                    CTL_HardSTOP(FAIL_RAMP); //hardstop will never return
                    break;
                }
#ifdef DEBUG_RHU
            case RHU_DISABLED_BY_SWITCH:
            case RHU_DISABLED:{
                    //this is an error, shouldn't happen
                    #ifdef DEBUG_RHU
                    printf("RHU_Watchdog_Service():: BUG - Disabled RHU being in watchdog\n");
                    #endif

                    break;
                }
#endif
            default:{
                    //Set lastramp to now
                    rhu_state[i].wd_lastramp = time_ms;
                }
            }
        }
    }

    //Check if the watchdog is finished (reset indicator and push recovery LCD message)
    if ( wd_active ) return;

    //Clear error message
    LED_Clear(LED_ONGOING);
    LcdPostModal(RECOVER);
}

