/*
 * LED.c
 *
 *  Created on: Mar 8, 2018
 *      Author: Zack Lyzen
 */

#include "LED.h"
#include "stdio.h"

//#define DEBUG_LED

static Uint16 ledPins[15];
static Uint16 ledWdPins[9];

void InitLeds(void)
{
	ledPins[LED_POWER]    = 0;
	ledPins[LED_MGMT]     = 101;
	ledPins[LED_M1] 	  = 0;
	ledPins[LED_ONGOING]  = 205;
	ledPins[LED_TCO]      = 204;
	ledPins[LED_TEMP]     = 203;
	ledPins[LED_MEZZ]     = 202;
	ledPins[LED_SFF_GRP]  = 201;
	ledPins[LED_M_2_GRP]  = 207; //200 changed to 207
	ledPins[LED_DIMM_GRP] = 107;
	ledPins[LED_RAM]      = 106;
	ledPins[LED_MISC]     = 105;
	ledPins[LED_CPU2]     = 104;
	ledPins[LED_CPU1]     = 103;
	ledPins[LED_M2]       = 0;

	ledWdPins[0] = ledPins[LED_CPU1];
	ledWdPins[1] = ledPins[LED_CPU2];
	ledWdPins[2] = ledPins[LED_MISC];
	ledWdPins[3] = ledPins[LED_RAM];
	ledWdPins[4] = ledPins[LED_DIMM_GRP];
	ledWdPins[5] = ledPins[LED_M_2_GRP];
	ledWdPins[6] = ledPins[LED_SFF_GRP];
	ledWdPins[7] = ledPins[LED_MEZZ];
	ledWdPins[8] = ledPins[LED_ONGOING];


	ExtGpioSet(ledPins[LED_MGMT], 0);
	ExtGpioSet(ledPins[LED_ONGOING], 0);
	ExtGpioSet(ledPins[LED_TCO], 0);
	ExtGpioSet(ledPins[LED_TEMP], 0);
	ExtGpioSet(ledPins[LED_MEZZ], 0);
	ExtGpioSet(ledPins[LED_SFF_GRP], 0);
	ExtGpioSet(ledPins[LED_M_2_GRP], 0);
	ExtGpioSet(ledPins[LED_DIMM_GRP], 0);
	ExtGpioSet(ledPins[LED_RAM], 0);
	ExtGpioSet(ledPins[LED_MISC], 0);
	ExtGpioSet(ledPins[LED_CPU2], 0);
	ExtGpioSet(ledPins[LED_CPU1], 0);
}

void SetLed(Uint16 req_)
{
#ifdef DEBUG_LED
    printf( "SetLed() called with req_=%d\n", req_);
#endif

    if ( req_ < 15 && ledPins[req_] ) ExtGpioSet(ledPins[req_], 1);
}

void ClearLed(Uint16 req_)
{
#ifdef DEBUG_LED
    printf( "ClearLed() called with req_=%d\n", req_);
#endif

    if ( req_ < 15 && ledPins[req_] ) ExtGpioSet(ledPins[req_], 0);
}

void LedWdSet(Uint16 Wd_)
{
#if DEBUG_LED
    printf( "LedWdSet() called with Wd_=%d\n", Wd_);
#endif

    if ( Wd_ < 9 && ledWdPins[Wd_] ) ExtGpioSet(ledWdPins[Wd_], 1);
}

void LedWdClear(Uint16 Wd_)
{
#if DEBUG_LED
    printf( "LedWdClear() called with Wd_=%d\n", Wd_);
#endif

    if ( Wd_ < 9 && ledWdPins[Wd_] ) ExtGpioSet(ledWdPins[Wd_], 0);
}
