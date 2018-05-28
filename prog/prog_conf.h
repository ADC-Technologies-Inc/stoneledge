/*
 * prog_conf.h
 *
 *  Created on: Feb 18, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_PROG_CONF_H_
#define PROG_PROG_CONF_H_

//
/*-----------------------------------------------------------------------------------------------------------
 *
 * General configurable settings for ADCt program
 *
 -----------------------------------------------------------------------------------------------------------*/

// lcd modes

#define 	STATIC 						0
#define 	MODAL 						1
#define 	CYCLE 						2

// lcd times

#define 	CYCLE_MSG_TIME 				8000
#define 	MODAL_MSG_TIME 				5000

// lcd messages

#define 	PRE_STARTUP 				0
#define 	STARTUP 					1
#define 	INIT_OK 					2
#define 	DELAY_STARTUP_FAIL_THERM 	3
#define 	INIT_FAIL_TCO_PTC 			4
#define 	INIT_FAIL_STARTUP 			5
#define 	FAIL_TCO_PTC 				6
#define 	FAIL_GEN_THERM 				7
#define 	OVERHEAT 					8
#define 	RECOVER 					9
#define     PREINIT_FAIL_BAD_READ       10
#define     INIT_BAD_48V                11
#define     FAIL_OVERHEAT_RHU_NOT_ON    12
#define     FAIL_RAMP                   13
#define     RAMP_RHU1                   14
#define     RAMP_RHU2                   15
#define     RAMP_RHU3                   16
#define     RAMP_RHU4                   17
#define     RAMP_RHU5                   18
#define     RAMP_RHU6                   19
#define     RAMP_RHU7                   20
#define     RAMP_RHU8                   21

// RHUs

#define 	CPU1 						0
#define 	CPU2 					    1
#define 	MISC 						2
#define 	RAM 						3
#define 	DIMM_GRP 					4
#define 	M_2_GRP 					5
#define 	SFF_GRP 					6
#define 	MEZZ 						7

//not RHUs but used by temp.c as error returns
#define 	ADDITIONAL_SENSORS 			8
#define     BOARD                       8
#define     AIR                         9

// LEDs- DO NOT CHANGE ORDER, LED.c is configured for LEDs to be listed in this order

#define 	LED_CPU1 					CPU1
#define 	LED_CPU2 					CPU2
#define 	LED_DIMM_GRP 				DIMM_GRP
#define 	LED_RAM 					RAM
#define 	LED_MEZZ 					MEZZ
#define 	LED_M_2_GRP 				M_2_GRP
#define 	LED_SFF_GRP 				SFF_GRP
#define     LED_MISC                    MISC
#define 	LED_TCO 					8
#define 	LED_ONGOING 				9
#define 	LED_TEMP 					10
#define 	LED_MGMT 					11
#define 	LED_POWER 					12
#define     LED_M1                      13
#define 	LED_M2 						14

// duty cycle switch configs

#define 	DUTY_0 						0
#define 	DUTY_50 					1
#define 	DUTY_75 					2
#define 	DUTY_100 					3

// Type Codes

#define 	TA 							00
#define 	TB 							01
#define 	TC 							02
#define 	AA 							03

// Timing Settings
// Enables

//#define 	USE_ETHERNET 	1						// uncomment to bring ethernet online (not ready)
//#define 	LOW_DUTY_MODE 	1						// uncomment to do limited testing without proper heatsinks
													// new duty cycles settings are 0, 5%, 7.5%, 10%
//#define     SHOW_DUTY       1                       // shows the duty % instead of the % towards ramping completion on the LCD
//#define     IGNORE_48VFUSE  1                       // uncomment to ignore a disconnected 48v fuse

#define     DISABLE_AIR 1                           //uncomment to allow the external temp sensor groups to disable the heaters


#define 	USE_RHU_1 	1 							// set to 0 to disabled RHU
#define 	USE_RHU_2 	1 							//
#define 	USE_RHU_3 	1 							//
#define 	USE_RHU_4 	1 							//
#define 	USE_RHU_5 	1 							//
#define 	USE_RHU_6 	1 							//
#define 	USE_RHU_7 	1 							//
#define 	USE_RHU_8 	1 							//

/*-----------------------------------------------------------------------------------------------------------
 *
 * Saftely limits -  Settings
 *
 -----------------------------------------------------------------------------------------------------------*/

////////////////////////////////////////
//	Board Current
////////////////////////////////////////

#define			MAX_PEAK_CURRENT		1800		// maximum peak board current A*100

////////////////////////////////////////
//	Temps for the various sensors (heaters do not share the same restrictions, neither do the VRM resistors are they're generally hardier on a board)
////////////////////////////////////////

#define     MAXTEMP_KAPTONHTR           1050
#define     MAXTEMP_VRM                 950
#define     MAXTEMP_CPU                 800
#define     MAXTEMP_RAM                 700
#define     MAXTEMP_DIMM                700
#define     MAXTEMP_M2                  700
#define     MAXTEMP_SFF                 700
#define     MAXTEMP_MEZZ                700
#define     MAXTEMP_MISC                850         //misc board components

#define 	PTC_DAMAGED_THRESHOLD_L 	80 			// raw ADC value that indicates a damaged permanent PTC
#define 	PTC_DAMAGED_THRESHOLD_U 	4000 		// raw ADC value that indicates a damaged permanent PTC
#define 	PTC_MISSING_THRESHOLD_L 	80 			// raw ADC value that indicates a missing removable PTC
#define 	PTC_MISSING_THRESHOLD_U 	4000 		// raw ADC value that indicates a missing removable PTC

////////////////////////////////////////
//  RHU RAMP
////////////////////////////////////////

#define     MIN_DUTY_TOCHECK_CYCLE      50          //minimum duty-cycle before a test of the RHU TCO/PTC will be performed, ensures that the optical isolaters states have been read ok
#define     RAMP_DELAY_CPU              10000       //delay between ramp cycles for CPU
#define     RAMP_DELAY                  5000        //delay between ramp cycles for other RHUs

////////////////////////////////////////
//  WATCHDOG
////////////////////////////////////////
#define     DISABLE_WATCHDOG            1           //disable the RHU watchdog which will not detect/trigger TCO fail errors
#define     RHU_WATCHDOG_DELAY          30000       //turn off each RHU for a minimum of 30s


////////////////////////////////////////
//  DEBUG SWITCHES
////////////////////////////////////////

//DEBUG needs to be defined if any of the DEBUG_xxx defines are defined
#ifndef DEBUG
#define DEBUG                                       //if DEBUG is set the ASSERT macro in ntd_debug.h are operational
#endif


#define DEBUG_I2C                       1           //i2c.c
#define DEBUG_CTL                       1           //ctl.c
#define DEBUG_RHU                       1           //RHU.c
//#define DEBUG_PWM                       1           //PWM.c
//#define DEBUG_IOINIT                    1           //IOInit.c
//#define DEBUG_LCD                       1           //LCD.c
//#define DEBUG_LED                       1           //LED.c
//#define DEBUG_SYS_ID                    1           //SysID.c
#define DEBUG_TEMPS                     1           //temps.c*/



#endif /* PROG_PROG_CONF_H_ */
