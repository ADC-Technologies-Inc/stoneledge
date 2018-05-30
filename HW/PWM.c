/*
 * PWM.c
 *
 *  Created on: Apr 4, 2016
 *      Author: zlyzen
 */

//===========================================================================
// Includes
//===========================================================================

//#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "PWM.h"
#include "../prog/ntd_debug.h"

#include "IQmathLib.h"
#include "../HW/ModuleConfig.h"

//===========================================================================
// ISR Prototypes
//===========================================================================

__interrupt void PWM_ePWM1_ISR__dutycycle(void);
__interrupt void PWM_ePWM4_ISR__analog(void);
__interrupt void PWM_ePWM5_ISR__tickcount(void);

#pragma CODE_SECTION(PWM_ePWM1_ISR__dutycycle, "ramfuncs");
#pragma CODE_SECTION(PWM_ePWM4_ISR__analog, "ramfuncs");
#pragma CODE_SECTION(PWM_ePWM5_ISR__tickcount, "ramfuncs");


//===========================================================================
// private variables
//===========================================================================
#define 	RHU_MAX 	8

static volatile Uint16 *PWM_DutyRegs[8]; 						// array of pointers to the duty cycle registers, for ease of use

/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Running
 *
 -----------------------------------------------------------------------------------------------------------*/

/*set_duty = duty% * 100. */
void PWM_SetDuty(uint16_t channel_, uint16_t set_duty_)
{
    #ifdef DEBUG_PWM
    printf("PWM_SetDuty():: Setting channel %u to %u\n", channel_, set_duty_);
    #endif
	*PWM_DutyRegs[channel_] = set_duty_;
}

/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Init
 *
 -----------------------------------------------------------------------------------------------------------*/

void PWM_EnISR(void)
{
    IER |= M_INT3;

    PieCtrlRegs.PIEIER3.bit.INTx1 = 0x01;
    PieCtrlRegs.PIEIER3.bit.INTx4 = 0x01;
    PieCtrlRegs.PIEIER3.bit.INTx5 = 0x01;
}

void PWM_MapISR(void)
{
	EALLOW;
	PieVectTable.EPWM1_INT = &PWM_ePWM1_ISR__dutycycle;
    PieVectTable.EPWM4_INT = &PWM_ePWM4_ISR__analog;
    PieVectTable.EPWM5_INT = &PWM_ePWM5_ISR__tickcount;
    EDIS;
}

/*
 * Clocks are configured as follows (spruge9e.pdf)
 *
 * TBCLK = SYSCLKOUT / (HSPCLKDIV × CLKDIV)
 *
 * CLKDIV has the following values for the relevant setting
 *
 * Setting  Value
 * 0        1
 * 1        2
 * 2        4
 * 3        8
 * 4        16
 * 5        32
 * 6        64
 * 7        128
 *
 * HSPCLKDIV has the following values for the relevant settings
 *
 * Setting  Value
 * 0        1
 * 1        2
 * 2        4
 * 3        6
 * 4        8
 * 5        10
 * 6        12
 * 7        14
 *
 *  SYSCLKOUT   CLKDIV  HSPCLKDIV   TBCLK       TBCLK Period    TBPRD   Time        Freq
 *  60,000,000  1       4           15,000,000  6.67E-08        10,000  6.67E-04    1500.00
 *  60,000,000  4       2           7,500,000   1.33E-07        10,000  1.33E-03    750.00
 *  60,000,000  4       4           3,750,000   2.67E-07        10,000  2.67E-03    375.00
 *  60,000,000  4       6           2,500,000   4.00E-07        10,000  4.00E-03    250.00
 *  60,000,000  4       8           1,875,000   5.33E-07        10,000  5.33E-03    187.50
 *  60,000,000  8       10          750,000     1.33E-06        10,000  1.33E-02    75.00
 *  60,000,000  16      12          312,500     3.20E-06        10,000  3.20E-02    31.25
 *  60,000,000  64      14          66,964      1.49E-05        10,000  1.49E-01    6.70
 *  60,000,000  128     12          39,063      2.56E-05        10,000  2.56E-01    3.91
 *  60,000,000  128     14          33,482      2.99E-05        10,000  2.99E-01    3.35
 *  60,000,000  128     10          46,875      2.13E-05        20,000  4.27E-01    2.34
 *  60,000,000  128     14          33,482      2.99E-05        16,741  5.00E-01    2.00
 *  60,000,000  128     14          33,482                      33,482              1.00
 *
 */

#define MOSFET_TBPRD        10000
#define MOSFET_HSPCLKDIV    0x07
#define MOSFET_CLKDIV       0x07

void PWM_Init()
{
	PWM_DutyRegs[0] = &EPwm1Regs.CMPA.half.CMPA;
	PWM_DutyRegs[1] = &EPwm1Regs.CMPB;
	PWM_DutyRegs[2] = &EPwm2Regs.CMPA.half.CMPA;
	PWM_DutyRegs[3] = &EPwm2Regs.CMPB;
	PWM_DutyRegs[4] = &EPwm3Regs.CMPA.half.CMPA;
	PWM_DutyRegs[5] = &EPwm3Regs.CMPB;
	PWM_DutyRegs[6] = &EPwm6Regs.CMPA.half.CMPA;
	PWM_DutyRegs[7] = &EPwm6Regs.CMPB;

   /////////////////////////////////
   //	ePWM1
   /////////////////////////////////
   EPwm1Regs.TBPRD 					= MOSFET_TBPRD;			// Period = 1000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	// with 60MHz clock and 1/2 prescale this should set the PWM to 30KHz
   EPwm1Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm1Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm1Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero // all rhu's on same phase
   EPwm1Regs.TBCTR 					= 0;					// clear TB counter
   EPwm1Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm1Regs.TBCTL.bit.PHSEN 		= 0;		     		// Phase loading disabled
   EPwm1Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm1Regs.TBCTL.bit.PHSDIR        = 1;                    // Count up after sync
   EPwm1Regs.TBCTL.bit.SYNCOSEL 	= TB_CTR_ZERO;			// output sync pulse when counter == 0
   EPwm1Regs.TBCTL.bit.HSPCLKDIV 	= MOSFET_HSPCLKDIV;		// TBCLK = SYSCLK/24
   EPwm1Regs.TBCTL.bit.CLKDIV 		= MOSFET_CLKDIV;		// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
   EPwm1Regs.CMPCTL.bit.SHDWAMODE 	= CC_SHADOW;			// shadow update for A
   EPwm1Regs.CMPCTL.bit.SHDWBMODE 	= CC_SHADOW;			// shadow update for B
   EPwm1Regs.CMPCTL.bit.LOADAMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm1Regs.CMPCTL.bit.LOADBMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm1Regs.AQCTLA.bit.ZRO 		= AQ_SET;				// Sets EPwm1A high on CTR = zero
   EPwm1Regs.AQCTLA.bit.CAU 		= AQ_CLEAR;				// Sets EPwm1A low on CTR = CMPA
   EPwm1Regs.AQCTLB.bit.ZRO 		= AQ_SET;				// Sets EPwm1B high on CTR = zero
   EPwm1Regs.AQCTLB.bit.CBU 		= AQ_CLEAR;				// Sets EPwm1B low on CTR = CMPB

   EPwm1Regs.ETSEL.bit.INTSEL       = 0x01; 				// trigger PWM1 interrupt everytime counter resets
   //EPwm1Regs.ETSEL.bit.INTEN 		= 0x01; 				// enable interrupt
   EPwm1Regs.ETPS.bit.INTPRD 		= 0x03; 				// every 3rd reset

   /////////////////////////////////
   //	ePWM2
   /////////////////////////////////
   EPwm2Regs.TBPRD 					= MOSFET_TBPRD;			    // Period = 1000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	// with 60MHz clock and 1/2 prescale this should set the PWM to 30KHz
   EPwm2Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm2Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm2Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero // all rhu's on same phase
   EPwm2Regs.TBCTR 					= 0;					// clear TB counter
   EPwm2Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm2Regs.TBCTL.bit.PHSEN 		= TB_ENABLE;		    // Phase loading enabled
   EPwm2Regs.TBCTL.bit.PHSDIR        = 1;                   // Count up after sync
   EPwm2Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm2Regs.TBCTL.bit.SYNCOSEL 	= TB_SYNC_IN;			// sync pulse input
   EPwm2Regs.TBCTL.bit.HSPCLKDIV 	= MOSFET_HSPCLKDIV;		// TBCLK = SYSCLK/24
   EPwm2Regs.TBCTL.bit.CLKDIV 		= MOSFET_CLKDIV;		// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
   EPwm2Regs.CMPCTL.bit.SHDWAMODE 	= CC_SHADOW;			// shadow update for A
   EPwm2Regs.CMPCTL.bit.SHDWBMODE 	= CC_SHADOW;			// shadow update for B
   EPwm2Regs.CMPCTL.bit.LOADAMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm2Regs.CMPCTL.bit.LOADBMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm2Regs.AQCTLA.bit.ZRO 		= AQ_SET;				// Sets EPwm2A high on CTR = zero
   EPwm2Regs.AQCTLA.bit.CAU 		= AQ_CLEAR;				// Sets EPwm2A low on CTR = CMPA
   EPwm2Regs.AQCTLB.bit.ZRO 		= AQ_SET;				// Sets EPwm2B high on CTR = zero
   EPwm2Regs.AQCTLB.bit.CBU 		= AQ_CLEAR;				// Sets EPwm2B low on CTR = CMPB
   /////////////////////////////////
   //	ePWM3
   /////////////////////////////////
   EPwm3Regs.TBPRD 					= MOSFET_TBPRD;			    // Period = 10000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	// with 60MHz clock and 1/2 prescale this should set the PWM to 30KHz
   EPwm3Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm3Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm3Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero // all rhu's on same phase
   EPwm3Regs.TBCTR 					= 0;					// clear TB counter
   EPwm3Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm3Regs.TBCTL.bit.PHSEN 		= TB_ENABLE;		    // Phase loading enabled
   EPwm3Regs.TBCTL.bit.PHSDIR       = 1;                    // Count up after sync
   EPwm3Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm3Regs.TBCTL.bit.SYNCOSEL 	= TB_SYNC_IN;			// sync pulse input
   EPwm3Regs.TBCTL.bit.HSPCLKDIV 	= MOSFET_HSPCLKDIV;		// TBCLK = SYSCLK/24
   EPwm3Regs.TBCTL.bit.CLKDIV 		= MOSFET_CLKDIV;		// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
   EPwm3Regs.CMPCTL.bit.SHDWAMODE 	= CC_SHADOW;			// shadow update for A
   EPwm3Regs.CMPCTL.bit.SHDWBMODE 	= CC_SHADOW;			// shadow update for B
   EPwm3Regs.CMPCTL.bit.LOADAMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm3Regs.CMPCTL.bit.LOADBMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm3Regs.AQCTLA.bit.ZRO 		= AQ_SET;				// Sets EPwm3A high on CTR = zero
   EPwm3Regs.AQCTLA.bit.CAU 		= AQ_CLEAR;				// Sets EPwm3A low on CTR = CMPA
   EPwm3Regs.AQCTLB.bit.ZRO 		= AQ_SET;				// Sets EPwm3B high on CTR = zero
   EPwm3Regs.AQCTLB.bit.CBU 		= AQ_CLEAR;				// Sets EPwm3B low on CTR = CMPB
   /////////////////////////////////
   //	ePWM6
   /////////////////////////////////
   EPwm6Regs.TBPRD 					= MOSFET_TBPRD;			    // Period = 10000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	    // with 60MHz clock and 1/2 prescale this should set the PWM to 30KHz
   EPwm6Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm6Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm6Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero
   EPwm6Regs.TBCTR 					= 0;					// clear TB counter
   EPwm6Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm6Regs.TBCTL.bit.PHSEN 		= TB_ENABLE;		    // Phase loading enabled
   EPwm6Regs.TBCTL.bit.PHSDIR       = 1;                    // Count up after sync
   EPwm6Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm6Regs.TBCTL.bit.SYNCOSEL 	= TB_SYNC_IN;			// sync pulse input
   EPwm6Regs.TBCTL.bit.HSPCLKDIV 	= MOSFET_HSPCLKDIV;		// /8  TBCLK = SYSCLK/24
   EPwm6Regs.TBCTL.bit.CLKDIV 		= MOSFET_CLKDIV;		// /4  Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
   EPwm6Regs.CMPCTL.bit.SHDWAMODE 	= CC_SHADOW;			// shadow update for A
   EPwm6Regs.CMPCTL.bit.SHDWBMODE 	= CC_SHADOW;			// shadow update for B
   EPwm6Regs.CMPCTL.bit.LOADAMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm6Regs.CMPCTL.bit.LOADBMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm6Regs.AQCTLA.bit.ZRO 		= AQ_SET;				// Sets EPWM6A high on CTR = zero
   EPwm6Regs.AQCTLA.bit.CAU 		= AQ_CLEAR;				// Sets EPWM6A low on CTR = CMPA
   EPwm6Regs.AQCTLB.bit.ZRO 		= AQ_SET;				// Sets EPWM6B high on CTR = zero
   EPwm6Regs.AQCTLB.bit.CBU 		= AQ_CLEAR;				// Sets EPWM6B low on CTR = CMPB

   /////////////////////////////////
   //	EPwm5
   /////////////////////////////////
   EPwm5Regs.TBPRD 					= 10000;			    // Period = 10000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	    // with 60MHz clock and 1/6 prescale this should set the PWM to 1KHz
   EPwm5Regs.CMPA.half.CMPA 		= 5000;			 	  	// Compare A = 5000 TBCLK counts // halfway through counting start ADC read
   EPwm5Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm5Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero
   EPwm5Regs.TBCTR 					= 0;					// clear TB counter
   EPwm5Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm5Regs.TBCTL.bit.PHSEN 		= 0;		     		// Phase loading disabled
   EPwm5Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm5Regs.TBCTL.bit.SYNCOSEL 	= TB_SYNC_DISABLE;		// No Sync
   EPwm5Regs.TBCTL.bit.HSPCLKDIV 	= 0x03;					// TBCLK = SYSCLK/6
   EPwm5Regs.TBCTL.bit.CLKDIV 		= 0x00;		 			// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
   EPwm5Regs.CMPCTL.bit.SHDWAMODE 	= CC_SHADOW;			// shadow update for A
   EPwm5Regs.CMPCTL.bit.SHDWBMODE 	= CC_SHADOW;			// shadow update for B
   EPwm5Regs.CMPCTL.bit.LOADAMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm5Regs.CMPCTL.bit.LOADBMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero

   EPwm5Regs.ETSEL.bit.INTSEL 		= 0x02; 				// when TBCTR reaches TBPRD an event will be triggered
   EPwm5Regs.ETPS.bit.INTPRD 		= 0x01; 				// every event where EPWM 4 reaches TBPRD will call EPwm5 interrupt (no event ps)
   EPwm5Regs.ETSEL.bit.INTEN 		= 0x01; 				// enable int EPwm5

   EPwm5Regs.ETSEL.bit.SOCASEL 		= 0x04; 				// when TBCTR == CMPA trigger an ADC read
   EPwm5Regs.ETPS.bit.SOCAPRD 		= 0x01; 				// every event triggers an ADC ready (1KHz sample rate)
   EPwm5Regs.ETSEL.bit.SOCAEN 		= 0x00; 				// don't trigger reads until ready

   /////////////////////////////////
   //	ePWM4
   /////////////////////////////////
   EPwm4Regs.TBPRD 					= 83;//16741;		    // Period = 5ms
   	   	   	   	   	   	   	   	   	   	   	   	   	   	    // with 60MHz clock and 1/(128*14) prescale this should set the PWM to 1 s
   EPwm4Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm4Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm4Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero
   EPwm4Regs.TBCTR 					= 0;					// clear TB counter
   EPwm4Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm4Regs.TBCTL.bit.PHSEN 		= 0;		     		// Phase loading disabled
   EPwm4Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm4Regs.TBCTL.bit.SYNCOSEL 	= TB_SYNC_DISABLE;		// No Sync
   EPwm4Regs.TBCTL.bit.HSPCLKDIV 	= 0x07;					// TBCLK = SYSCLK/1792
   EPwm4Regs.TBCTL.bit.CLKDIV 		= 0x07;		 			// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
   EPwm4Regs.CMPCTL.bit.SHDWAMODE 	= CC_SHADOW;			// shadow update for A
   EPwm4Regs.CMPCTL.bit.SHDWBMODE 	= CC_SHADOW;			// shadow update for B
   EPwm4Regs.CMPCTL.bit.LOADAMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero
   EPwm4Regs.CMPCTL.bit.LOADBMODE 	= CC_CTR_ZERO; 			// load [new duty cycle from shadow register] on CTR = zero

   EPwm4Regs.ETSEL.bit.INTSEL 		= 0x02; 				// when TBCTR reaches TBPRD an event will be triggered
   EPwm4Regs.ETPS.bit.INTPRD 		= 0x01; 				// every event where EPWM 4 reaches TBPRD will call EPwm4 interrupt (no event ps)
   //EPwm4Regs.ETSEL.bit.INTEN 		= 0x01; 				// enable int EPwm4
   return;
}

/*-----------------------------------------------------------------------------------------------------------
 *
 * ISRs
 *
 -----------------------------------------------------------------------------------------------------------*/

//Highest priority group 3 vector, called when the duty cycle starts- measure if RHUs are on here
__interrupt void  PWM_ePWM1_ISR__dutycycle(void)
{
    static uint16_t count = 15;
    //ENABLE i2c and ePWM5 interrupts while in this one (group 8, 3)

    uint16_t TempPIEIER3;
    TempPIEIER3 = PieCtrlRegs.PIEIER3.all;      // .. 3
    IER |= M_INT3;                              // re-enable group 3 PIE (ePWM5)
    IER |= M_INT8;                              // re-enable group 8 PIE (i2C)

    PieCtrlRegs.PIEIER3.all = 0x0000;           // disable all but ePWM5
    PieCtrlRegs.PIEIER3.bit.INTx5 = 0x01;       // ePWM5 interrupt enabled ....
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;     // ack this group to allow more group 3 ints to fire


    asm(" NOP");                                // wait one cycle
    EINT;                                       // re-enable global interrupts

    /*Callback runs here, only every 16th time round*/
    if (count==15){
        RHU_PWMCallback();
        count = 0;
    }else count++;

	// Clear INT flag for this timer
	EPwm1Regs.ETCLR.bit.INT = 1;

	// Acknowledge this interrupt to receive more interrupts from group 3 (redundant)
	//PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

    DINT;
    PieCtrlRegs.PIEIER3.all = TempPIEIER3;
}

//disabled until this is called
void PWM_EnableAnalogISR(void){
    EPwm4Regs.ETSEL.bit.INTEN   = 0x01;
}

/*timer for processing analog data and servicing LCD*/
__interrupt void  PWM_ePWM4_ISR__analog(void)
{
    static uint16_t event = 0;
    static uint16_t service = 0;
    static uint16_t reset_sw = 0;

	////
	// 	changes to PIEIER1 and PIEIER8 cannot happen inside this ISR (group 3) so changes to them are commented out
	////
	uint16_t TempPIEIER3;
	TempPIEIER3 = PieCtrlRegs.PIEIER3.all; 		// .. 3
	IER = M_INT1; 								// re-enable group 1 PIE (?)
	IER |= M_INT3; 								// re-enable group 3 PIE (ePWM1 and ePWM5)
	IER |= M_INT8; 								// re-enable group 8 PIE (i2C)
	PieCtrlRegs.PIEIER3.all = 0x0000; 			// all group 3 except as requried disabled
	PieCtrlRegs.PIEIER3.bit.INTx1 = 0x01;       // ePWM1 interrupt enabled (TCO verify) //we enable ePWM1 to allow the best chance of picking up accurate TCO bits, there is a slight possibility that it may have an issue though
	PieCtrlRegs.PIEIER3.bit.INTx5 = 0x01; 		// ePWM5 interrupt enabled (ms counter)
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;     // let loose

	asm(" NOP"); 								// wait one cycle
	EINT; 										// re-enable global interrupts

	////
	// 	start normal interrupt code

	switch(event){
	case 0: { StartAnalog(); event++; break; }              //Start data collection in present channel (should take 10ms)
    case 3: { event++; ProcessAnalogResult(); break; }      //process result and switch channel
	case 4: { event = 0; break; }                           //Wait to allow channel to settle
	default: event++;
	}

	/*
	 * Timer is presently set to 5ms
	 */

	//Debounce our input switch
	if (GpioDataRegs.GPADAT.bit.GPIO21 == 0){
	    reset_sw++;

	    //20 = 100ms held
	    if (reset_sw == 20){
	        CTL_HardSTOP(RESET_BUTTON);
	    }
	}else reset_sw = 0;

	//Service every 500ms including
	if (service++ == 100){
	    if (online) CTL_OnlineCALLBACK();

        LcdService();
        service = 0;
	}

	// Clear INT flag for this timer
	EPwm4Regs.ETCLR.bit.INT = 1;

	////
	// 	end normal interrupt code
	////

	DINT;
	PieCtrlRegs.PIEIER3.all = TempPIEIER3;
	//GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;
}

__interrupt void  PWM_ePWM5_ISR__tickcount(void)
{
	//GpioDataRegs.GPASET.bit.GPIO0 = 1;
	// EPWM5 takes care of the time (in ms)

    //no point incurring the call overhead to increment a single var that should be atomic anyway.
	//increment_time_ms();
    time_ms++;

	// Clear INT flag for this timer
	EPwm5Regs.ETCLR.bit.INT = 1;
	// Acknowledge this interrupt to receive more interrupts from group 3
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
	//GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
}
