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
//
// NTD - moved InitLeds() call to perform sweep without interruption by watchdog

#include "adct.h"

uint16_t progMode = PRE_STARTUP_MODE;

void InitLoop(void);

void InitializeProgram(void)
{

	DisableRhuRelay(); 						// Turn off relay load block
	ExtGpioInit(); 							// Initialized eGPIOs
    InitLeds();                             // initilze LEDS (off) // startup sweep?
    InitSysId();                            // read system ID

	InitializeRHUs(); 						// Initialize RHUS
	InitRhuWatchdog(); 						// Init Rhu Watchdog

	InitNtc(); 								// initialize analog/ptc section
	InitCurrent(); 							// initialize analog/current sense section
	InitMux(); 								// initializes ADC MUX
#ifdef USE_LCD
	LcdInit(); 								// initialize/clear LCD screen
	// post startup LCD message
#endif
	//StartupTest(); 						// cycle relays, analog mux, and screen

	///////////////////////////////
	// Individual RHU Test Start
	///////////////////////////////
	/*
	EPwm2Regs.CMPB = 7500; 					// 10% duty cycle
	EnableRhuRelay();
	delay_ms(30000); 						// wait 10 seconds
	EPwm2Regs.CMPB = 0; 					// 0% ducty cycle
	delay_ms(1000);
	DisableRhuRelay();
	 */
	///////////////////////////////
	// Individual RHU Test End
	///////////////////////////////
	InitLoop(); 							// initializes timer that calls Control Loop function
	StartAnalog(); 							// starts ADC reads



#ifdef USE_ETHERNET
	// initialize ethenet section
#endif
	// start temp reads // interrupt
}

void StartupTest(void)
{
	// checks that all TCOs read low voltage
	// checks that fuse reads good power
	// checks that relay is off,
	// turns on relay
	// checks that relay is on
	// checks that TCOs read good
	// turns off relay
	// checks that relay is off
	// checks that TCOs read good
}

void InitLoop(void)
{
	EPwm4Regs.ETSEL.bit.INTEN 	= 0x01;
}

void ControlLoop(void)
{
	ProcessAnalogResult(); 					// read analog temps
	ProcessTempData(); 						// check for ptc ready
	EvaluateTempData(); 					// checks for watchdog issues
	CheckTcoResults(); 						// check TCO/PTC - these are checked on a separte timer to align with PWM
	UpdateRhuWatchdog(); 					// check for added watchdog
	ServiceRhuWatchdog(); 					// process watchdog
	StartAnalog();							// analog results have been processed and ADCMUX has been incremented, enough settling time has passed re-enable ADC reads
	SetMode(); 								// set new Mode if necessary

	if(CheckFuse() == 0) 					// if fuse is blown
	{
		EstopRhu();
		// if hard-stop kick out of loop and shut down
	}
	if(CritialError() == 1) 				// if cricital error has occured
	{
		EstopRhu();
		// if hard-stop kick out of loop and shut down
	}
}

void SetMode(void)
{
	switch (progMode) {
	case PRE_STARTUP_MODE : 				//
		if(TIME_MS > PRE_STARTUP_DELAY)
		{
			if(GetRhuWatchdog(8) == 1)
			{
				reset_time_ms();
			}
			else
			{
				progMode = STARTUP_MODE;
			}
		}
		break;
	case STARTUP_MODE : 					// bring rhus on slowly
		if(TIME_MS > STARTUP_DELAY)
		{
			progMode = RUNNING_MODE;
		}
		break;
	case RUNNING_MODE : 					// check for critical failure
		if(CritialError() == 1)
		{
			progMode = CRITICAL_FAILURE_MODE;
		}
		break;
	default : 								// assume critical failure mode

		 break;
	}
}

Uint16 GetMode(void)
{
	return progMode;
}

