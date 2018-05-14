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
// modes
#define 	PRE_STARTUP_MODE 			0
#define 	STARTUP_MODE 				1
#define 	RUNNING_MODE 				2
#define 	CRITICAL_FAILURE_MODE 		3

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

#define 	PRE_STARTUP_DELAY 			5000 		// delay (ms)
#define 	STARTUP_DELAY 				9000 		//
#define 	MAIN_LOOP_MS 				1000 		// (ms * 1000) sets timing for main loop (monitoring board, not ethernet tx)

// Enables

//#define 	USE_ETHERNET 							// uncomment to bring ethernet online (not ready)
#define 	USE_LCD 								// uncomment to bring LCD online
#define 	BYPASS_TCO_ERROR 						// uncomment to stop triggering TCO fail errors
#define 	LOW_DUTY_MODE 							// uncomment to do limited testing without proper heatsinks
													// new duty cycles settings are 0, 5%, 7.5%, 10%

#define 	USE_RHU_1 	0 							// set to 0 to disabled RHU
#define 	USE_RHU_2 	0 							//
#define 	USE_RHU_3 	0 							//
#define 	USE_RHU_4 	0 							//
#define 	USE_RHU_5 	1 							//
#define 	USE_RHU_6 	0 							//
#define 	USE_RHU_7 	0 							//
#define 	USE_RHU_8 	0 							//

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
//	RHU Temp
////////////////////////////////////////

#define 	AIR_TEMP_MAX_LIMIT 			700 		// temp value (c * 10) before error is thrown
#define 	BOARD_TEMP_MAX_LIMIT 		700			// Board temp limit
#define     RHU1_TEMP_MAX_LIMIT         700         // CPU1
#define     RHU2_TEMP_MAX_LIMIT         700         // CPU2
#define 	RHU_TEMP_MAX_LIMIT 			700         // All other RHUs


#define 	PTC_DAMAGED_THRESHOLD_L 	80 			// raw ADC value that indicates a damaged permanent PTC
#define 	PTC_DAMAGED_THRESHOLD_U 	4000 		// raw ADC value that indicates a damaged permanent PTC
#define 	PTC_MISSING_THRESHOLD_L 	80 			// raw ADC value that indicates a missing removable PTC
#define 	PTC_MISSING_THRESHOLD_U 	4000 		// raw ADC value that indicates a missing removable PTC

////////////////////////////////////////
//  RHU RAMP
////////////////////////////////////////

#define     MIN_DUTY_TOCHECK_CYCLE      50
#define     RAMP_DELAY_CPU              10000
#define     RAMP_DELAY                  5000
#define     RHU_WATCHDOG_DELAY              30000       //turn off each RHU for a minimum of 30s

#endif /* PROG_PROG_CONF_H_ */
