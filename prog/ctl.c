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

void InitLoop(void);

void InitializeProgram(void)
{
    //Make sure 48V system is off
	RHU_Disable48V();

	//Reset ext GPIOs
	ExtGpioInit();

	//Put some info on the screen
    LcdInit();
    LcdPostStatic(PRE_STARTUP);

    //Do our sweep and initialize LEDs
    LED_InitLeds();

    //Pickup SystemID
    InitSysId();

    //Get RHUs ready to go (read duty)
	RHU_Init();

	//Initialize the analog sections
	InitNtc(); 								// initialize analog/ptc section
	InitCurrent(); 							// initialize analog/current sense section
	InitMux(); 								// initializes ADC MUX

#ifdef USE_ETHERNET
	// initialize ethenet section
#endif


	//Enter Control Loop
	ControlLoop_PreInit();
}

//TODO REMOVE VESTIGIAL
/*void InitLoop(void)
{
	EPwm4Regs.ETSEL.bit.INTEN 	= 0x01;
}*/

void ControlLoop_PreInit(void){
    //Pre-init
    uint32_t entry_time;
    int ret;
    uint16_t retry = 0, delayed = 0, i;

    //Collect sensor data for 5s only
re_enter:
    entry_time = get_time_ms();

    while( (get_time_ms() - entry_time) > 5000 ){

        //Start Analog Collection
        StartAnalog();

        delay_ms(10);

        //Process and shift to next channel
        ProcessAnalogResult();

        delay_ms(50);
    }

    //five seconds have past, check if all thermistors are within limit
    ret = ProcessTempData();
    if (ret < 0 ){
        //PRE INIT - Temps not ready, this really shouldn't happen...; try once more
        if ( retry ){
             //Hard Stop
            CTL_HardSTOP(PREINIT_FAIL_BAD_READ);

            //System should idle at this point.
        }

        retry++;

        //omg a goto! wtf - bad programmer, bad! :P
        goto re_enter;
    }else if ( ret > 0 ){
        //PRE INIT - Over Temperature condition, re-enter until it comes within spec.
        delayed = ret;

        LcdPostStatic( DELAY_STARTUP_FAIL_THERM );  //post message to LCD
        LED_Set(LED_ONGOING);                       //set ongoing LED

        for (i = 0; i < 8; i++ ){
            if ( ret & 0x1 ) LED_Set(i);
            ret = ret >> 1;
        }
        if ( ret ) LED_Set(LED_TEMP);

        goto re_enter;
    }

    //Temps all good, move on to Init Phase
    if (delayed){

        //Clear any set LEDs
        LED_Clear(LED_ONGOING);
        for (i=0; i< 8; i++) LED_Clear(i);
        LED_Clear(LED_TEMP);
    }

    ControlLoop_Init();
}

void ControlLoop_Init(void){

    /*GOT HERE*/

    StartAnalog();                          // start ADC reads up
    PWM_EnableAnalogISR();                  // enable the background ADC timer



	//ProcessAnalogResult(); 					// read analog temps


	ProcessTempData(); 						// check for ptc ready
	EvaluateTempData(); 					// checks for watchdog issues
	RHU_VerifyRHU(); 						// check TCO/PTC - these are checked on a different timer to align with PWM
	UpdateRhuWatchdog(); 					// check for added watchdog
	ServiceRhuWatchdog(); 					// process watchdog
	StartAnalog();							// analog results have been processed and ADCMUX has been incremented, enough settling time has passed re-enable ADC reads
	SetMode(); 								// set new Mode if necessary

	if(CheckFuse() == 0) 					// if fuse is blown
	{
		RHU_EStopRHU();
		// if hard-stop kick out of loop and shut down
	}
	if(CriticalError() == 1) 				// if cricital error has occured
	{
		RHU_EStopRHU();
		// if hard-stop kick out of loop and shut down
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
}
