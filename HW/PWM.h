/*
 * PWM.h
 *
 *  Created on: Apr 4, 2016
 *      Author: zlyzen
 */

#ifndef PWM_H_
#define PWM_H_

//===========================================================================
// Includes
//===========================================================================
#include <stdint.h>
#include "../bits.h"
#include "DSP2803x_Device.h"     // DSP2803x Headerfile Include File

#include "../prog/prog_conf.h"
#include "../prog/rhu.h"
#include "../prog/time.h"
#include "../HW/Interface.h"
#include "../prog/ctl.h"
#include "../prog/lcd.h"        // LCD is serviced by ePWM4
#include "DSP28x_Project.h"

//===========================================================================
// Function Prototypes
//===========================================================================
// running
// duty cycle
// channel_
// 1 = EPWM1A
// 2 = EPWM1B
// 3 = EPWM2A
// 4 = EPWM2B
// 5 = EPWM3A
// 6 = EPWM3B
// 7 = EPWM6A
// 8 = EPWM6B
// req_ = (duty cycle % * 10)
void PWM_SetDuty(uint16_t channel_, uint16_t req_);
// init
void PWM_EnISR(void);
void PWM_MapISR(void);
void PWM_Init(void); 									// initializes PWM peripherial with desired settings for EPWM 1, 2, 3, & 6 used by RHU1-8
void PWM_EnableAnalogISR(void);                            //enables the analog ePWM isr
#endif /* PWM_H_ */
