/*
 * Analog.h
 *
 *  Created on: Apr 26, 2016
 *      Author: zlyzen
 */

#ifndef ANALOG_H_
#define ANALOG_H_

//===========================================================================
// Includes
//===========================================================================

#include "../HW/ADC.h"				 // ADC header file
#include "../HW/ModuleConfig.h"
#include "../HW/PWM.h"
#include "../prog/current.h"
#include "../prog/ntc.h"

//===========================================================================
// Function Prototypes
//===========================================================================

#define FLAG_START  1
#define FLAG_END    0
void AnalogDiscard(uint16_t flag_);

void ProcessAnalogResult(void);
void StartAnalog(void);
void StopAnalog(void);

void ADCISREn(void);
void ADCISRMap(void);
void ConfigADC(void);
void InitializeAnalog(void);
uint16_t* GetAnalogAddress(uint16_t req_);


#endif /* ANALOG_H_ */
