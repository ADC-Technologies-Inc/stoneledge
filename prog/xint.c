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
    XIntruptRegs.XINT1CR.bit.POLARITY = 0;
    XIntruptRegs.XINT1CR.bit.ENABLE = 1;

    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;         // Enable PIE Group 1, INT 4 (XINT1)
    IER |= M_INT1;                             // Enable CPU INT1
}

//The interrupt
__interrupt void XINT_eInt1(void){
    //Bring the system down with a lovely message (all we really want is to be able to turn it off...)
    CTL_HardSTOP(RESET_BUTTON);

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

#endif /* PROG_XINT_C_ */
