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

#define 	RHU_COUNT 			8

//===========================================================================
// External Facing Functions
//===========================================================================

//Initializer, call before any of the other funcs
void RHU_Init(void);

//Callback from ePWM1 ISR, do not call
void RHU_PWMCallback( void );

//48V System
void RHU_Enable48V(void);
void RHU_Disable48V(void);
Uint16 RHU_Get48VFuse(void);

//Verify RHUs are powered on and operating as intended, if an enabled RHU is down this will halt the system.
void RHU_VerifyRHU(void);

//Enable/Disable RHUs
void RHU_EnableRHU(uint16_t rhu_);
void RHU_DisableRHU(uint16_t rhu_);

//Emergency Stop, brings down 48V and all RHUs
void RHU_EStopRHU(void);

#endif /* PROG_RHU_H_ */
