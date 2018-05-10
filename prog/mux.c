/*
 * mux.c
 *
 *  Created on: Mar 9, 2018
 *      Author: Zack Lyzen
 */

#include "mux.h"

uint16_t currentMux = 0;

void InitMux(void)
{
	SetMux(currentMux);
}

uint16_t GetMux(void)
{
	return currentMux;
}

void SetMux(uint16_t channel_)
{
	currentMux = channel_;

	if(currentMux & BIT0)
		 ExtGpioSet(507, 1);
	else
		 ExtGpioSet(507, 0);

	if(currentMux & BIT1)
		ExtGpioSet(501, 1);
	else
		ExtGpioSet(501, 0);

	if(currentMux & BIT2)
		ExtGpioSet(502, 1);
	else
		ExtGpioSet(502, 0);

	if(currentMux & BIT3)
		ExtGpioSet(503, 1);
	else
		ExtGpioSet(503, 0);
}
