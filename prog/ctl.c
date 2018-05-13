/*
 * adct.c
 *
 *  Created on: Mar 7, 2018
 *      Author: Zack Lyzen
 */

//
// R0.20 Updates
//
// PWM.c
// Corrected bugs with nested interrupts for EPWM4 timer interrupt
//
// fixed dividers + period for EPWM registers 4 + 5 (proper timing)
//
// I2C.c
// fixed error in I2C check_hold function
//
// lcd_driver.c
// fixed error in lcd init function
// added clear screen command to lcd init function
//
// lcd.c
// fixed errors for set temps and set power in lcd.c
//
// changed ptc.c to ntc.c (typo)
// need to change names of ptc functions to ntc
// added proper math for ntc.c resistance to temperature calcualtion
//
//
// R0.30 Updates
//
// PWM.c
// fixed issue where EPWM5 wasn't triggering ADC reads to start
//
// I2C.c
// fixed issue where NACK caused a hang, now complete transaction is retried and has been running without snags
//
// lcd.c
// fixed issue where lcd messages that were cycling weren't rhu messages but error messages
// fixed issue where timing of rhu/modal messages could be off
//
// temps.c
// fixed issue with raw (averaged) ADC value being converted to temp
// updated contents of struct for each rhu temp
//
// ntc.c
// fixed names that included ptc to be ntc throughout code
// fixed function ProcessNtcSet(set_) which was modifying an array out-of-range causing important memory values to change
//
// R0.40 Updates
//
// Hardware
// discovered that ADC_MUX_EN was disconnected.  Pulled up to 3v3A or 3v3 to get ADC_MUX working properly
// removed resistors for LED M1 and M2
// discovered that the RHU MOSFET gate drive buffers and relay MOSFET gate drive buffers have floating enable pins, grounded
//
// added small bit of code in InitializeProgram() to test individual RHUs for a short period of time.  (commented out)
//
// temps.c
// fixed error regarding frequency of temperature calculations
//

#include "ctl.h"
#include "ntd_debug.h"

#define CTL_DEBUG

//Internal funcs.
static void CTL_PreInit(void);
static void CTL_Init(void);
static void CTL_Online(void);

/*Enter the main program loop*/
void CTL_Enter(void){
    uint16_t i;

    //Make sure 48V system is off
	RHU_Disable48V();

    //Reset ext GPIOs
    ExtGpioInit();

    //Read Duty Configs and make sure all RHUs are in disabled position
    RHU_Init();
	for (i=0;i<RHU_COUNT;i++){
	    RHU_DisableRHU(i);
	}

	//Put some info on the screen
    LcdInit();
    LcdPostStatic(PRE_STARTUP);

    //Do our sweep and initialize LEDs
    LED_InitLeds();

    //Pickup SystemID
    InitSysId();

	//Initialize the analog sections
	InitNtc(); 								// initialize analog/ptc section
	InitCurrent(); 							// initialize analog/current sense section
	MUX_Init(); 								// initializes ADC MUX

#ifdef USE_ETHERNET
	// initialize ethernet section
#endif


	//Enter Control Loop
	CTL_PreInit();
}

/*Perform pre-init tasks*/
void CTL_PreInit(void){
    //Pre-init
    //uint32_t entry_time;
    int ret;
    uint16_t retry = 0, delayed = 0, i;

    //Start collecting sensor data
    PWM_EnableAnalogISR();

re_enter:
    //entry_time = get_time_ms();

    delay_ms(5000);

/*    while( (get_time_ms() - entry_time) > 5000 ){

        //Start Analog Collection
        StartAnalog();

        delay_ms(20);

        //Process and shift to next channel
        ProcessAnalogResult();

        delay_ms(10);
    }*/

    //five seconds has passed, check if all thermistors are within limit
    ret = ProcessTempData();
    #ifdef CTL_DEBUG
    printf("CTL_PreInit():: Processed Temp Data returned %d\n", ret );
    #endif

    if (ret < 0 ){
        //PRE INIT - Temps not ready, this really shouldn't happen...; try once more
        if ( retry ){
            #ifdef CTL_DEBUG
            printf("CTL_PreInit():: Temps not ready after retry, calling HardSTOP\n" );
            #endif

             //Hard Stop
            CTL_HardSTOP(PREINIT_FAIL_BAD_READ);
            //System should idle at this point.
        }

        #ifdef CTL_DEBUG
        printf("CTL_PreInit():: Temps not ready, retrying\n" );
        #endif

        retry++;

        //omg a goto! wtf - bad programmer, bad! :P
        goto re_enter;
    }else if ( ret > 0 ){
        //PRE INIT - Over Temperature condition, re-enter until it comes within spec.
        delayed = ret;

        LcdPostStatic( DELAY_STARTUP_FAIL_THERM );  //post message to LCD
        LED_Set(LED_ONGOING);                       //set ongoing LED

        //Set necessary RHU LEDs
        for (i = 0; i < 8; i++ ){
            if ( ret & 0x1 ) LED_Set(i);
            ret = ret >> 1;

            #ifdef CTL_DEBUG
            printf("CTL_PreInit():: Over temperature error on RHU %d\n", i );
            #endif

        }
        if ( ret ) LED_Set(LED_TEMP);

        goto re_enter;
    }

    //Temps all good, move on to Init Phase
    if (delayed){
        #ifdef CTL_DEBUG
        printf("CTL_PreInit():: Temps good, clearing delayed LEDs\n" );
        #endif

        //Clear any set LEDs
        LED_Clear(LED_ONGOING);
        for (i=0; i< 8; i++) LED_Clear(i);
        LED_Clear(LED_TEMP);
    }

    CTL_Init();
}

/*Perform init tasks*/
void CTL_Init(void){
    int rhu = 0, ramp_ret = 0, ret, i;

    LcdPostStatic( STARTUP );

    /*We need to power on the relay and the RHUs, relay first*/
    RHU_Enable48V();
    delay_ms(10);

    /*Check power is on*/
    if ( !RHU_Get48VFuse() ) {
        #ifdef CTL_DEBUG
        printf("CTL_Init():: 48V Fuse fail, calling HardSTOP\n" );
        #endif

        CTL_HardSTOP(INIT_BAD_48V);
    }

    /*Bring up the RHUs - we do this slowly, ramping up to the full duty-cycle over a several minutes to avoid potential inrush current issues*/
    for ( rhu = 0; rhu < RHU_COUNT; ){

        /*bring up the RHUs in order*/
        ramp_ret = RHU_EnableRHU_RAMP(rhu);
        switch(ramp_ret){
        case RHU_FULLPOWER: { rhu++; break; }
        case RHU_ABORT: {
            /*Fail on an RHU of some form, this will probably never be returned but it's an abort anyway*/
            LED_Set(rhu);
            CTL_HardSTOP(FAIL_RAMP); //hardstop will never return
        }
        case RHU_DISABLED_BY_SWITCH:
        case RHU_DISABLED:
            rhu++;
        default:
            //RHU ok, we should have the present duty_cycle in ramp_ret
            ASSERT( ramp_ret >= 0 );
        }

        if ( ramp_ret > MIN_DUTY_TOCHECK_CYCLE ){
            //Check the RHU is powered, this will halt if there's an issue
            RHU_VerifyRHU(INIT_FAIL_TCO_PTC);
        }

        //delay before re-entering ramp
        delay_ms( rhu > 1 ? RAMP_DELAY : RAMP_DELAY_CPU );

        ret = ProcessTempData();
        if (ret > 0){
            //Overtemp, call HardSTOP (aka. there's something wrong that isn't minor; this isn't your normal overheat if it happened almost immediately!)
            //Set necessary RHU LEDs
            for (i = 0; i < RHU_COUNT; i++ ){
                if ( ret & 0x1 ) LED_Set(i);
                ret = ret >> 1;

                #ifdef CTL_DEBUG
                printf("CTL_Init():: Over temperature error on RHU %d\n", i );
                #endif

                //re-enter for loop
                continue;
            }

            //ELSE board or air overheat
            if ( ret ) LED_Set(LED_TEMP);

            CTL_HardSTOP(INIT_FAIL_STARTUP); //hardstop will never return
        }

    }

    //RHUs all online, system is green, startup complete
    #ifdef CTL_DEBUG
    printf("CTL_Init():: Startup Complete - All Systems 5x5\n" );
    #endif

    //Illuminate the celebratory LED
    LED_Set(LED_MGMT);
    LcdPostModal(INIT_OK);

    CTL_Online();
}

//Entering online mode, pretty simple- run at 2hz and do stuff.
#define LOOP_TIME     500
void CTL_Online(){
    uint32_t last_entry, time_to_fin;
    uint16_t i;
    int ret;

    while (1)
    {
        last_entry = time_ms;

        //Check the RHUs are powered, this will halt if there's an issue
        RHU_VerifyRHU(FAIL_TCO_PTC);

        //Check temps
        ret = ProcessTempData();
        if (ret > 0){

            //Over temperature condition in an RHU or on board
            for (i = 0; i < 8; i++ ){
                if ( ret & 0x1 ) LED_Set(i);
                ret = ret >> 1;

                #ifdef CTL_DEBUG
                printf("CTL_Online():: Over temperature error on RHU %d, reporting to watchdog\n", i );
                #endif

                RHU_Watchdog_FAIL(i);
            }

            if ( ret ){
                //Watchdog can't help, the problem is elsewhere - shutdown.
                #ifdef CTL_DEBUG
                printf("CTL_Online():: Over temperature error on board or air, bringing down system\n" );
                #endif

                LED_Set(LED_TEMP);
                CTL_HardSTOP(FAIL_GEN_THERM);
            }
        }


        #ifdef USE_ETHERNET                         // if use Ethernet is selected
        ServiceEthernet();              // service ethernet every 1ms - though sends every 500ms
        #endif

        //Service Watchdog
        RHU_Watchdog_Service();

        //Service LCD
        LcdService();

        //Hold for next loop
        time_to_fin = time_ms - last_entry;
        if ( time_to_fin >= LOOP_TIME ) continue;

        delay_ms(time_to_fin-LOOP_TIME);
    }


}

/*HARDSTOP*/
void CTL_HardSTOP( uint16_t msg_){
    RHU_EStopRHU();                         //bring down 48v subsystem
    LcdPostStatic( msg_ );                  //post message to LCD
    LED_Set(LED_ONGOING);                   //set ongoing LED

    //Disable interrupts
    DINT;

    //Bring the system to a halt
    EALLOW;
    SysCtrlRegs.LPMCR0.bit.LPM = 0x0002;   // LPM mode = Halt
    EDIS;

    // Force device into HALT
    __asm(" IDLE");

    //in case it didn't work
    while(1) asm(" NOP");
}
