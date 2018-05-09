/*
 * ADC.h
 *
 *  Created on: Apr 5, 2016
 *      Author: zlyzen
 */

#ifndef ADC_H_
#define ADC_H_

#include "DSP2803x_Device.h"     // DSP2803x Headerfile Include File
#include "DSP2803x_Examples.h"   // DSP2803x Examples Include File

//===========================================================================
// Defines
//===========================================================================
#define ADC_usDELAY  1000L


//===========================================================================
// Function Prototypes
//===========================================================================

void InitAdc(void);

void InitAdcAio(void);


#endif /* ADC_H_ */
