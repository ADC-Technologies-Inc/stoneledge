/*
 * xint.c
 *
 *  Created on: 30 May 2018
 *      Author: ntd
 */

#ifndef PROG_XINT_C_
#define PROG_XINT_C_

#include "DSP2803x_Device.h"     // DSP2803x Headerfile Include File
#include "xint.h"
#include "ctl.h"
#include "prog_conf.h"
#include "../prog/time.h"

__interrupt void XINT_eInt1(void);

//Register the interrupt
void XINT_Init( void ){
    EALLOW;

    //Select GPIO21 as our reset switch connected GPIO
    GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 21;

    PieVectTable.XINT1 = &XINT_eInt1;
    EDIS;
}

//Configure and enable the interrupt, call after IOInit has setup
void XINT_Enable( void ){
    XIntruptRegs.XINT1CR.bit.POLARITY = 3;      //interrupt on both falling and rising edge
    XIntruptRegs.XINT1CR.bit.ENABLE = 1;

    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;         // Enable PIE Group 1, INT 4 (XINT1)
    IER |= M_INT1;                             // Enable CPU INT1
}

//The interrupt
__interrupt void XINT_eInt1(void){
    //If the button press lasts more than 2 seconds, we halt- otherwise we ignore it as bounce or a transient
    static uint32_t press = 0;

    if (press == 0){
        //Start the counter
        press = time_ms;
    }else{
        //How long was the button held for?
        if ( (time_ms - press ) < 2000 ){
            //less than 2s, discard
            press = 0;
        }else{
            //more than 2s, shutdown
            press = 0;
            CTL_HardSTOP(RESET_BUTTON);
        }
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

#endif /* PROG_XINT_C_ */
