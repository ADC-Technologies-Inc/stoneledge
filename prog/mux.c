/*
 * mux.c
 *
 *  Created on: Mar 9, 2018
 *      Author: Zack Lyzen
 */

#include "mux.h"
#include "../HW/IOInit.h"

//#define MUX_DEBUG

#ifdef MUX_DEBUG
#include "ntd_debug.h"
#endif

static uint16_t currentMux = 0;

void MUX_Init(void)
{
	MUX_Set(currentMux);
}

uint16_t MUX_Get(void)
{
	return currentMux;
}

void MUX_Set(uint16_t channel_)
{
	uint16_t gpio_bits = channel_ & 0xE; //only bits 0x2,0x4,0x8 can be copied directly
	currentMux = channel_;

	//As ExtGPIO 500 is setup with 4 input pins and 4 output pins we can write all 4 pins in one communication without knowing the state of the 4 input pins as the mux will ignore what we write to the output port for pins configured as inputs
	if (currentMux & 0x1) gpio_bits |= 0x80;    //turn on last bit

    #ifdef MUX_DEBUG
	printf("MUX_Set():: Setting MUX to channel %d, writing bits: "PRINTF_BINSTR8"\n", channel_,PRINTF_BINSTR8_ARGS(gpio_bits) );
    #endif

	GpioSetDirArray(5,gpio_bits);
}
