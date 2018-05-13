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
void RHU_VerifyRHU(uint16_t fail_msg_);

//Enable/Disable RHUs
void RHU_EnableRHU(uint16_t rhu_);
void RHU_DisableRHU(uint16_t rhu_);

/*
 *
 * Slowly ramp up the RHU to full power, this function should be called repeatedly
 *
 * RHU_FULLPOWER will be returned when the RHU is fully powered up
 * RHU_ABORT will be returned when there was an error powering up the RHU (not a TCO error)
 * RHU_DISABLED_BY_SWITCH will be returned when the RHU is disabled by the duty-cycle being set to 0
 * RHU_DISABLED will be returned when the RHU is disabled by another non-fatal means
 *
 * otherwise the current duty_cycle of the RHU will be returned
 *
 */

#define RHU_FULLPOWER               0x7FFF
#define RHU_ABORT                   -1
#define RHU_DISABLED_BY_SWITCH      -2
#define RHU_DISABLED                -3
int RHU_EnableRHU_RAMP(uint16_t rhu_);

//Emergency Stop, brings down 48V and all RHUs
void RHU_EStopRHU(void);

//Watchdog functions, use fail to report an overtemp; use service to allow the watchdog to handle it
void RHU_Watchdog_FAIL(uint16_t rhu_);
void RHU_Watchdog_Service(void);

#endif /* PROG_RHU_H_ */
