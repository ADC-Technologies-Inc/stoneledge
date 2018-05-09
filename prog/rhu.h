/*
 * rhu.h
 *
 *  Created on: Feb 25, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_RHU_H_
#define PROG_RHU_H_

//===========================================================================
// Includes
//===========================================================================
#include <stdint.h>
#include "../bits.h"
#include "DSP2803x_Device.h"     // DSP2803x Headerfile Include File

#include "../HW/PWM.h"
#include "../HW/IOInit.h"
#include "rhu_watchdog.h"

//===========================================================================
// Defines
//===========================================================================

#define 	RHU_QTY 			8
#define 	RHU_RELAY_STATE 	GetRhuRelayState()

//===========================================================================
// Function Prototypes
//===========================================================================
// running
void EnableRhuRelay(void);
void DisableRhuRelay(void);
Uint16 GetRhuRelayState(void);
void CheckTco(Uint16 tco_, Uint16 duty_cycle_);
void CheckTcoResults(void);
void ProcessRhuRunning(void); 							// checks each rhu to see if enabled/disabled, sets duty accordingly
void EnableRhu(uint16_t rhu_); 							// enables the specified rhu - changes take effect next time "ProcessRhuRunning" is called
void DisableRhu(uint16_t rhu_); 						// disables the specified rhu - changes take effect next time "ProcessRhuRunning" is called
void EstopRhu(void); 									// immediately sets all duty cycles to 0% and disables all Rhus
// init
void SetRhuDutyArray(uint16_t rhu_, uint16_t duty_); 	// sets the "on" duty cycle for specified RHu
void InitializeRHUs(void); 								//

#endif /* PROG_RHU_H_ */
