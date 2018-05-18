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

//Global vars.
int online = 0;

//Internal funcs.
static void CTL_PreInit(void);
static void CTL_Init(void);
static void CTL_OnlineBGTasks(void);

/*Enter the main program loop*/
void CTL_Enter(void){
    uint16_t i;

    //Make sure 48V system is off
    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Turning off 48v sub-system\n" );
    #endif

	RHU_Disable48V();

    //Reset ext GPIOs
    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Initializing External GPIO\n" );
    #endif

    ExtGpioInit();

    //Read Duty Configs and make sure all RHUs are in disabled position
    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Initializing RHU code and disabling RHU\n" );
    #endif

    RHU_Init();
	for (i=0;i<RHU_COUNT;i++){
	    RHU_DisableRHU(i);
	}

	//Put some info on the screen
    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Initializing LCD\n" );
    #endif

    LcdInit();
    LcdPostStatic(PRE_STARTUP);
    LcdService();

    //Do our sweep and initialize LEDs
    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Doing LED sweep\n" );
    #endif

    LED_InitLeds();

    //Pickup SystemID
    InitSysId();
    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Initializing SysID to "PRINTF_BINSTR8"\n", PRINTF_BINSTR8_ARGS( GetSysId() ) );
    #endif

	//Initialize the analog sections
    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Initializing Analog PTC code\n" );
    #endif
	InitNtc(); 								// initialize analog/ptc section

    #ifdef DEBUG_CTL
	printf("CTL_Enter():: Initializing current code\n" );
    #endif
	InitCurrent(); 							// initialize analog/current sense section

    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Initializing ADC Mux code\n" );
    #endif
	MUX_Init(); 								// initializes ADC MUX

#ifdef USE_ETHERNET
	// initialize ethernet section
#endif


	//Enter Control Loop
    #ifdef DEBUG_CTL
    printf("CTL_Enter():: Entering PreInit phase\n" );
    #endif

	CTL_PreInit();
}

/*Perform pre-init tasks*/
void CTL_PreInit(void){
    //Pre-init
    //uint32_t entry_time;
    int ret;
    uint16_t retry = 0, delayed = 0, i;

    //Start collecting sensor data
    #ifdef DEBUG_CTL
    printf("CTL_PreInit():: Enabling Analog ISRs (ePWM4)\n" );
    #endif

    PWM_EnableAnalogISR();

re_enter:
    //entry_time = get_time_ms();
    #ifdef DEBUG_CTL
    printf("CTL_PreInit():: Pausing for 5 seconds while temps are collected\n" );
    #endif
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
    #ifdef DEBUG_CTL
    switch(ret){
    case -1: {printf("CTL_PreInit():: Processing Temp Data returned NOT_READY\n" ); break;}
    case 0: {printf("CTL_PreInit():: Processing Temp Data returned TEMPS_OK\n" ); break;}
    default:{
            if (ret & 0x1) printf("CTL_PreInit():: Processing Temp Data returned CPU1 overheat\n" );
            if (ret & 0x2) printf("CTL_PreInit():: Processing Temp Data returned CPU2 overheat\n" );
            if (ret & 0x4) printf("CTL_PreInit():: Processing Temp Data returned MISC overheat\n" );
            if (ret & 0x8) printf("CTL_PreInit():: Processing Temp Data returned RAM overheat\n" );
            if (ret & 0x10) printf("CTL_PreInit():: Processing Temp Data returned DIMM_GRP overheat\n" );
            if (ret & 0x20) printf("CTL_PreInit():: Processing Temp Data returned M_2_GRP overheat\n" );
            if (ret & 0x40) printf("CTL_PreInit():: Processing Temp Data returned SFF_GRP overheat\n" );
            if (ret & 0x80) printf("CTL_PreInit():: Processing Temp Data returned MEZZ overheat\n" );
            if (ret & 0x100) printf("CTL_PreInit():: Processing Temp Data returned BOARD overheat\n" );
            if (ret & 0x200) printf("CTL_PreInit():: Processing Temp Data returned AIR overheat\n" );
        }
    }
    #endif

    if (ret < 0 ){
        //PRE INIT - Temps not ready, this really shouldn't happen...; try once more
        if ( retry ){
            #ifdef DEBUG_CTL
            printf("CTL_PreInit():: Temps not ready after retry, calling HardSTOP\n" );
            #endif

             //Hard Stop
            CTL_HardSTOP(PREINIT_FAIL_BAD_READ);
            //System should idle at this point.
        }

        #ifdef DEBUG_CTL
        printf("CTL_PreInit():: Temps not ready, retrying\n" );
        #endif

        retry++;

        //omg a goto! wtf - bad programmer, bad! :P
        goto re_enter;
    }else if ( ret > 0 ){
        //PRE INIT - Over Temperature condition, re-enter until it comes within spec.
        delayed = ret;

        #ifdef DEBUG_CTL
        printf("CTL_PreInit():: Posting DELAY_STARTUP_FAIL_THERM and setting LED_ONGOING\n" );
        #endif
        LcdPostStatic( DELAY_STARTUP_FAIL_THERM );  //post message to LCD
        LcdService();
        LED_Set(LED_ONGOING);                       //set ongoing LED

        //Set necessary RHU LEDs
        for (i = 0; i < RHU_COUNT; i++ ){
            if ( ret & 0x1 ) LED_Set(i);
            ret = ret >> 1;

            #ifdef DEBUG_CTL
            printf("CTL_PreInit():: Over temperature error on RHU %d\n", i );
            #endif

        }
        if ( ret ) LED_Set(LED_TEMP);

        goto re_enter;
    }

    //Temps all good, move on to Init Phase
    if (delayed){
        #ifdef DEBUG_CTL
        printf("CTL_PreInit():: Temps good, clearing delayed LEDs\n" );
        #endif

        //Clear any set LEDs
        LED_Clear(LED_ONGOING);
        for (i=0; i< 8; i++) LED_Clear(i);
        LED_Clear(LED_TEMP);
    }

    #ifdef DEBUG_CTL
    printf("CTL_PreInit():: Entering Init Phase\n" );
    #endif
    CTL_Init();
}

/*Perform init tasks*/
void CTL_Init(void){
    int rhu = 0, ramp_ret = 0, ret, i;


    #ifdef DEBUG_CTL
    printf("CTL_Init():: Posting STARTUP Message\n" );
    #endif
    LcdPostStatic( STARTUP );
    LcdService();

    /*Check power is available*/
    if ( !RHU_Get48VFuse() ) {
        #ifdef DEBUG_CTL
        printf("CTL_Init():: 48V Fuse fail, calling HardSTOP\n" );
        #endif

        #ifndef IGNORE_48VFUSE
        CTL_HardSTOP(INIT_BAD_48V);
        #endif
    }

    /*We need to power on the relay and the RHUs, relay first*/
    #ifdef DEBUG_CTL
    printf("CTL_Init():: Enabling 48v system\n" );
    #endif
    RHU_Enable48V();
    delay_ms(10);

    /*Bring up the RHUs - we do this slowly, ramping up to the full duty-cycle over a several minutes to avoid potential inrush current issues*/
    #ifdef DEBUG_CTL
    printf("CTL_Init():: Entering RHU Ramp Upm\n" );
    #endif

    //Enable watchdog ISR, to disable RHU Watchdog comment this line out
    #ifndef DISABLE_WATCHDOG
    EPwm1Regs.ETSEL.bit.INTEN = 0x01;
    #endif

    for ( rhu = 0; rhu < RHU_COUNT; ){

        /*bring up the RHUs in order*/
        #ifdef DEBUG_CTL
        printf("CTL_Init():: Enabling 48v system\n" );
        #endif

        ramp_ret = RHU_EnableRHU_RAMP(rhu);
        switch(ramp_ret){
        case RHU_FULLPOWER: {
            #ifdef DEBUG_CTL
            printf("CTL_Init():: RHU %d at Full Power, moving on\n", rhu );
            #endif

            LcdPostStaticRamp( RAMP_RHU1+rhu, RHU_GetSetDutyPerc(rhu) );

            rhu++;
            break;
        }
        case RHU_ABORT: {
            /*Fail on an RHU of some form, this will probably never be returned but it's an abort anyway*/
            #ifdef DEBUG_CTL
            printf("CTL_Init():: RHU %d ABORT\n", rhu );
            #endif

            LED_Set(rhu);
            CTL_HardSTOP(FAIL_RAMP); //hardstop will never return
        }
        case RHU_DISABLED_BY_SWITCH:
        case RHU_DISABLED:{
            #ifdef DEBUG_CTL
            printf("CTL_Init():: RHU %d disabled, moving on\n", rhu );
            #endif

            rhu++;
            continue;   //re-enter, nothing to be done for this RHU
            }
        default:
            //RHU ok, we should have the present duty_cycle in ramp_ret
            #ifdef DEBUG_CTL
            printf("CTL_Init():: RHU %d Ramp OK, present duty = %d\n", rhu, ramp_ret );
            #endif
            ASSERT( ramp_ret >= 0 );

            LcdPostStaticRamp( RAMP_RHU1+rhu, ramp_ret );
        }

        //Post ramping message

        if ( ramp_ret > MIN_DUTY_TOCHECK_CYCLE ){
            //Check the RHU is powered, this will halt if there's an issue
            #ifdef DEBUG_CTL
            printf("CTL_Init():: Checking TCO/PTC\n");
            #endif

            RHU_VerifyRHU(INIT_FAIL_TCO_PTC);
        }

        //delay before re-entering ramp
        #ifdef DEBUG_CTL
        printf("CTL_Init():: Delaying %d ms until next ramp\n", (rhu > 1 ? RAMP_DELAY : RAMP_DELAY_CPU) );
        #endif

        /*delay and service LCD*/
        delay_ms( rhu > 1 ? RAMP_DELAY : RAMP_DELAY_CPU );

        ret = ProcessTempData();
        #ifdef DEBUG_CTL
        switch(ret){
        case -1: {printf("CTL_Init():: Processing Temp Data returned NOT_READY\n" ); break;}
        case 0: {printf("CTL_Init():: Processing Temp Data returned TEMPS_OK\n" ); break;}
        default:{
                if ((ret & 0x1)) printf("CTL_Init():: Processing Temp Data returned CPU1 overheat\n" );
                if ((ret & 0x2)) printf("CTL_Init():: Processing Temp Data returned CPU2 overheat\n" );
                if ((ret & 0x4)) printf("CTL_Init():: Processing Temp Data returned MISC overheat\n" );
                if ((ret & 0x8)) printf("CTL_Init():: Processing Temp Data returned RAM overheat\n" );
                if ((ret & 0x10)) printf("CTL_Init():: Processing Temp Data returned DIMM_GRP overheat\n" );
                if ((ret & 0x20)) printf("CTL_Init():: Processing Temp Data returned M_2_GRP overheat\n" );
                if ((ret & 0x40)) printf("CTL_Init():: Processing Temp Data returned SFF_GRP overheat\n" );
                if ((ret & 0x80)) printf("CTL_Init():: Processing Temp Data returned MEZZ overheat\n" );
                if ((ret & 0x100)) printf("CTL_Init():: Processing Temp Data returned BOARD overheat\n" );
                if ((ret & 0x200)) printf("CTL_Init():: Processing Temp Data returned AIR overheat\n" );
            }
        }
        #endif

        if (ret > 0){
            //Overtemp, call HardSTOP (aka. there's something wrong that isn't minor; this isn't your normal overheat if it happened almost immediately!)
            //Set necessary RHU LEDs
            for (i = 0; i < RHU_COUNT; i++ ){
                if ( ret & 0x1 ) LED_Set(i);
                ret = ret >> 1;

                #ifdef DEBUG_CTL
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
    #ifdef DEBUG_CTL
    printf("CTL_Init():: Startup Complete - All Systems 5x5\n" );
    #endif

    //Illuminate the celebratory LED
    #ifdef DEBUG_CTL
    printf("CTL_Init():: Posting INIT_OK Message and turning on Management Light\n" );
    #endif
    LED_Set(LED_MGMT);
    LcdPostModal(INIT_OK);


    #ifdef DEBUG_CTL
    printf("CTL_Init():: Entering Online Phase\n" );
    #endif
    online = 1;

    //The main cpu task handles the background tasks, such as Ethernet service
    CTL_OnlineBGTasks();
}

void CTL_OnlineBGTasks(){
    while(1){
        //service Ethernet here, placeholder for now.
        delay_ms(65535);
    }
}

//Online mode callback from ePWM4
void CTL_OnlineCALLBACK(){
    uint16_t i;
    int ret;

    //Check the RHUs are powered, this will halt if there's an issue
    #ifdef DEBUG_CTL
    printf("CTL_OnlineCALLBACK():: Checking TCO/PTC\n");
    #endif
    RHU_VerifyRHU(FAIL_TCO_PTC);

    //Check temps
    ret = ProcessTempData();
    #ifdef DEBUG_CTL
    switch(ret){
    case -1: {printf("CTL_OnlineCALLBACK():: Processing Temp Data returned NOT_READY\n" ); break;}
    case 0: {printf("CTL_OnlineCALLBACK():: Processing Temp Data returned TEMPS_OK\n" ); break;}
    default:{
            if ( (ret & 0x1) ) printf("CTL_OnlineCALLBACK():: Processing Temp Data returned CPU1 overheat\n" );
            if ( (ret & 0x2) ) printf("CTL_OnlineCALLBACK():: Processing Temp Data returned CPU2 overheat\n" );
            if ( (ret & 0x4) ) printf("CTL_OnlineCALLBACK():: Processing Temp Data returned MISC overheat\n" );
            if ( (ret & 0x8) ) printf("CTL_Online():: Processing Temp Data returned RAM overheat\n" );
            if ( (ret & 0x10) ) printf("CTL_Online():: Processing Temp Data returned DIMM_GRP overheat\n" );
            if ( (ret & 0x20) ) printf("CTL_Online():: Processing Temp Data returned M_2_GRP overheat\n" );
            if ( (ret & 0x40) ) printf("CTL_Online():: Processing Temp Data returned SFF_GRP overheat\n" );
            if ( (ret & 0x80) ) printf("CTL_Online():: Processing Temp Data returned MEZZ overheat\n" );
            if ( (ret & 0x100) ) printf("CTL_Online():: Processing Temp Data returned BOARD overheat\n" );
            if ( (ret & 0x200) ) printf("CTL_Online():: Processing Temp Data returned AIR overheat\n" );
        }
    }
    #endif

    if (ret > 0){

        //Over temperature condition in an RHU or on board
        for (i = 0; i < RHU_COUNT; i++ ){
            if ( (ret & 0x1) ){
                LED_Set(i);
                #ifdef DEBUG_CTL
                printf("CTL_OnlineCALLBACK():: Over temperature error on RHU %d, reporting to watchdog\n", i );
                #endif

                RHU_Watchdog_FAIL(i);
            }

            ret >>= 1;
        }

        if ( ret ){
            //Watchdog can't help, the problem is elsewhere - shutdown.
            #ifdef DEBUG_CTL
            printf("CTL_OnlineCALLBACK():: Over temperature error on board or air, bringing down system\n" );
            #endif

            LED_Set(LED_TEMP);
            CTL_HardSTOP(FAIL_GEN_THERM);
        }
    }

    //Service Watchdog
    #ifdef DEBUG_CTL
    printf("CTL_Online():: Checking TCO/PTC\n");
    #endif
    RHU_Watchdog_Service();
}

/*HARDSTOP*/
void CTL_HardSTOP( uint16_t msg_){
    RHU_EStopRHU();                         //bring down 48v subsystem
    LcdPostStatic( msg_ );                  //post message to LCD
    LcdService();
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
    while(1) delay_ms(65535);
}
