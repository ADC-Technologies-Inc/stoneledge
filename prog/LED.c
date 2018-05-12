/*
 * LED.c
 *
 *  Created on: Mar 8, 2018
 *      Author: Zack Lyzen
 *
 *
 *  N. Davidson
 *  - added LED sweep
 *  - refactored
 */

#include "LED.h"
#include "stdio.h"
#include "DSP2803x_Examples.h"

//#define DEBUG_LED

/*PRIVATE VARS*/
struct LED_Pins_ {
    uint16_t pin;               //pin GPIO
    uint16_t state;             //pin state (on/off)
}; 

struct LED_Pins_ LED_Pins[15];// = {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0};

//static Uint16 LED_Pins_ByName[15];
//static Uint16 LED_Pins_ByRHU[9];



/*PRIVATE FUNCTIONS*/
static void LED_Sweep( void );
static void LED_SweepBase(int dir_, int set_);

/*FUNCTIONS*/
void LED_InitLeds(void)
{    
	LED_Pins[LED_POWER].pin    = 0;
	LED_Pins[LED_MGMT].pin     = 101;
	LED_Pins[LED_M1].pin       = 0;
	LED_Pins[LED_ONGOING].pin  = 205;
	LED_Pins[LED_TCO].pin      = 204;
	LED_Pins[LED_TEMP].pin     = 203;
	LED_Pins[LED_MEZZ].pin     = 202;
	LED_Pins[LED_SFF_GRP].pin  = 201;
	LED_Pins[LED_M_2_GRP].pin  = 207; //200 changed to 207 to fix board bug
	LED_Pins[LED_DIMM_GRP].pin = 107;
	LED_Pins[LED_RAM].pin      = 106;
	LED_Pins[LED_MISC].pin     = 105;
	LED_Pins[LED_CPU2].pin     = 104;
	LED_Pins[LED_CPU1].pin     = 103;
	LED_Pins[LED_M2].pin       = 0;

	ExtGpioSet(LED_Pins[LED_MGMT].pin, 0);
	ExtGpioSet(LED_Pins[LED_ONGOING].pin, 0);
	ExtGpioSet(LED_Pins[LED_TCO].pin, 0);
	ExtGpioSet(LED_Pins[LED_TEMP].pin, 0);
	ExtGpioSet(LED_Pins[LED_MEZZ].pin, 0);
	ExtGpioSet(LED_Pins[LED_SFF_GRP].pin, 0);
	ExtGpioSet(LED_Pins[LED_M_2_GRP].pin, 0);
	ExtGpioSet(LED_Pins[LED_DIMM_GRP].pin, 0);
	ExtGpioSet(LED_Pins[LED_RAM].pin, 0);
	ExtGpioSet(LED_Pins[LED_MISC].pin, 0);
	ExtGpioSet(LED_Pins[LED_CPU2].pin, 0);
	ExtGpioSet(LED_Pins[LED_CPU1].pin, 0);

	LED_Sweep();
}

void LED_Set(Uint16 name_){
#ifdef DEBUG_LED
    printf( "LED_Set() called with name_=%d\n", name_);
#endif

    if ( name_ < 15 && LED_Pins[name_].pin && !LED_Pins[name_].state ) {
        ExtGpioSet(LED_Pins[name_].pin, 1);
        LED_Pins[name_].state = 1;
    }
}

void LED_Clear(Uint16 name_){
#ifdef DEBUG_LED
    printf( "LED_Clear() called with name_=%d\n", name_);
#endif

    if ( name_ < 15 && LED_Pins[name_].pin && LED_Pins[name_].state ) {
        ExtGpioSet(LED_Pins[name_].pin, 0);
        LED_Pins[name_].state = 0;
    }
}

void LED_Sweep( void ){
   LED_SweepBase(0, 1);
   LED_SweepBase(0, 0);
   LED_SweepBase(0, 1);
   LED_SweepBase(0, 0);
}

#define LED_SWEEP_DELAY 40000

void LED_SweepBase(int dir_, int set_){
    int i;

    for (\
            i = (dir_) ? 0 : 11\
            ; (dir_) ? i < 12 : i >= 0\
            ; (dir_) ? i++ : i--\
            ){
        switch (i){
        case 0: { ExtGpioSet(LED_Pins[LED_MGMT].pin, set_); break; }
        case 1: { ExtGpioSet(LED_Pins[LED_ONGOING].pin, set_); break; }
        case 2: { ExtGpioSet(LED_Pins[LED_TCO].pin, set_); break; }
        case 3: { ExtGpioSet(LED_Pins[LED_TEMP].pin, set_); break; }
        case 4: { ExtGpioSet(LED_Pins[LED_MEZZ].pin, set_); break; }
        case 5: { ExtGpioSet(LED_Pins[LED_SFF_GRP].pin, set_); break; }
        case 6: { ExtGpioSet(LED_Pins[LED_M_2_GRP].pin, set_); break; }
        case 7: { ExtGpioSet(LED_Pins[LED_DIMM_GRP].pin, set_); break; }
        case 8: { ExtGpioSet(LED_Pins[LED_RAM].pin, set_); break; }
        case 9: { ExtGpioSet(LED_Pins[LED_MISC].pin, set_); break; }
        case 10: { ExtGpioSet(LED_Pins[LED_CPU2].pin, set_); break; }
        case 11: { ExtGpioSet(LED_Pins[LED_CPU1].pin, set_); break; }
        }
        DELAY_US(LED_SWEEP_DELAY);
    }
}
