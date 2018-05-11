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

#include "IQmathLib.h"
#include "../HW/ModuleConfig.h"


//===========================================================================
// ISR Prototypes
//===========================================================================

__interrupt void EPwm1_timer_isr(void);
__interrupt void EPwm4_timer_isr(void);
__interrupt void EPwm5_timer_isr(void);

#pragma CODE_SECTION(EPwm1_timer_isr, "ramfuncs");
#pragma CODE_SECTION(EPwm4_timer_isr, "ramfuncs");
#pragma CODE_SECTION(EPwm5_timer_isr, "ramfuncs");


//===========================================================================
// private variables
//===========================================================================
#define 	RHU_MAX 	8

static volatile Uint16 *duty_cycle_registers[8]; 						// array of pointers to the duty cycle registers, for ease of use
static uint16_t pwm_1_count = 0;

/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Running
 *
 -----------------------------------------------------------------------------------------------------------*/

void SetDuty(uint16_t channel_, uint16_t req_)
{
	*duty_cycle_registers[channel_-1] = req_;
}

/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Init
 *
 -----------------------------------------------------------------------------------------------------------*/

void PWMISREn(void)
{
    IER |= M_INT3;

    PieCtrlRegs.PIEIER3.bit.INTx1 = 0x01;
    PieCtrlRegs.PIEIER3.bit.INTx4 = 0x01;
    PieCtrlRegs.PIEIER3.bit.INTx5 = 0x01;
}

void PWMISRMap(void)
{
	EALLOW;
	PieVectTable.EPWM1_INT = &EPwm1_timer_isr;
    PieVectTable.EPWM4_INT = &EPwm4_timer_isr;
    PieVectTable.EPWM5_INT = &EPwm5_timer_isr;
    EDIS;
}

void InitEPwm()
{
	duty_cycle_registers[0] = &EPwm1Regs.CMPA.half.CMPA;
	duty_cycle_registers[1] = &EPwm1Regs.CMPB;
	duty_cycle_registers[2] = &EPwm2Regs.CMPA.half.CMPA;
	duty_cycle_registers[3] = &EPwm2Regs.CMPB;
	duty_cycle_registers[4] = &EPwm3Regs.CMPA.half.CMPA;
	duty_cycle_registers[5] = &EPwm3Regs.CMPB;
	duty_cycle_registers[6] = &EPwm6Regs.CMPA.half.CMPA;
	duty_cycle_registers[7] = &EPwm6Regs.CMPB;

   /////////////////////////////////
   //	ePWM1
   /////////////////////////////////
   EPwm1Regs.TBPRD 					= 10000;			     	// Period = 1000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	// with 60MHz clock and 1/2 prescale this should set the PWM to 30KHz
   EPwm1Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm1Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm1Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero // all rhu's on same phase
   EPwm1Regs.TBCTR 					= 0;					// clear TB counter
   EPwm1Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm1Regs.TBCTL.bit.PHSEN 		= 0;		     		// Phase loading disabled
   EPwm1Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm1Regs.TBCTL.bit.SYNCOSEL 	= TB_CTR_ZERO;			// output sync pulse when counter == 0
   EPwm1Regs.TBCTL.bit.HSPCLKDIV 	= 0x03;					// TBCLK = SYSCLK/24
   EPwm1Regs.TBCTL.bit.CLKDIV 		= 0x02;		 			// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
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
   EPwm2Regs.TBPRD 					= 10000;			     	// Period = 1000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	// with 60MHz clock and 1/2 prescale this should set the PWM to 30KHz
   EPwm2Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm2Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm2Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero // all rhu's on same phase
   EPwm2Regs.TBCTR 					= 0;					// clear TB counter
   EPwm2Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm2Regs.TBCTL.bit.PHSEN 		= TB_ENABLE;		    // Phase loading enabled
   EPwm2Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm2Regs.TBCTL.bit.SYNCOSEL 	= TB_SYNC_IN;			// sync pulse input
   EPwm2Regs.TBCTL.bit.HSPCLKDIV 	= 0x03;					// TBCLK = SYSCLK/24
   EPwm2Regs.TBCTL.bit.CLKDIV 		= 0x02;		 			// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
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
   EPwm3Regs.TBPRD 					= 10000;			     	// Period = 1000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	// with 60MHz clock and 1/2 prescale this should set the PWM to 30KHz
   EPwm3Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm3Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm3Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero // all rhu's on same phase
   EPwm3Regs.TBCTR 					= 0;					// clear TB counter
   EPwm3Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm3Regs.TBCTL.bit.PHSEN 		= TB_ENABLE;		    // Phase loading enabled
   EPwm3Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm3Regs.TBCTL.bit.SYNCOSEL 	= TB_SYNC_IN;			// sync pulse input
   EPwm3Regs.TBCTL.bit.HSPCLKDIV 	= 0x03;					// TBCLK = SYSCLK/24
   EPwm3Regs.TBCTL.bit.CLKDIV 		= 0x02;		 			// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
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
   EPwm6Regs.TBPRD 					= 10000;			     	// Period = 1000 TBCLK counts
   	   	   	   	   	   	   	   	   	   	   	   	   	   	    // with 60MHz clock and 1/2 prescale this should set the PWM to 30KHz
   EPwm6Regs.CMPA.half.CMPA 		= 0;			 	  	// Compare A = 0 TBCLK counts
   EPwm6Regs.CMPB 					= 0;					// Compare B = 0 TBCLK counts
   EPwm6Regs.TBPHS.half.TBPHS 		= 0;					// Set Phase register to zero
   EPwm6Regs.TBCTR 					= 0;					// clear TB counter
   EPwm6Regs.TBCTL.bit.CTRMODE 		= 0;			  		// Count up
   EPwm6Regs.TBCTL.bit.PHSEN 		= TB_ENABLE;		    // Phase loading enabled
   EPwm6Regs.TBCTL.bit.PRDLD 		= 0;			   		// Shadow mode (Immediate mode == 1)
   EPwm6Regs.TBCTL.bit.SYNCOSEL 	= TB_SYNC_IN;			// sync pulse input
   EPwm6Regs.TBCTL.bit.HSPCLKDIV 	= 0x03;					// TBCLK = SYSCLK/24
   EPwm6Regs.TBCTL.bit.CLKDIV 		= 0x02;		 			// Used for prescale ( TBCLK = SYSCLKOUT / ( HSPCLKDIV x CLKDIV ) )
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
   EPwm4Regs.TBPRD 					= 16741;				// Period = 1000 ms
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
__interrupt void  EPwm1_timer_isr(void)
{
#ifndef BYPASS_TCO_ERROR
	CheckTco(pwm_1_count, *duty_cycle_registers[pwm_1_count]);
#endif /* BYPASS_TCO_ERROR*/
	pwm_1_count++;
	if(pwm_1_count >= RHU_MAX)
		pwm_1_count = 0;

	// Clear INT flag for this timer
	EPwm1Regs.ETCLR.bit.INT = 1;
	// Acknowledge this interrupt to receive more interrupts from group 3
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void  EPwm4_timer_isr(void)
{
	////
	// 	changes to PIEIER1 and PIEIER8 cannot happen inside this ISR (group 3) so changes to them are commented out
	////
	//GpioDataRegs.GPASET.bit.GPIO1 = 1;
	uint16_t TempPIEIER3;
	TempPIEIER3 = PieCtrlRegs.PIEIER3.all; 		// .. 3
	IER = M_INT1; 								// re-enable group 1 PIE
	IER |= M_INT3; 								// re-enable group 3 PIE
	IER |= M_INT8; 								// re-enable group 8 PIE
	PieCtrlRegs.PIEIER3.all = 0x0000; 			// ePWM1 interrupt enabled ....
	PieCtrlRegs.PIEIER3.bit.INTx5 = 0x01; 		// ePWM5 interrupt enabled ....



	asm(" NOP"); 								// wait one cycle
	EINT; 										// re-enable global interrupts
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

	////
	// 	start normal interrupt code
	////

	ControlLoop();

	// Clear INT flag for this timer
	EPwm4Regs.ETCLR.bit.INT = 1;
	// Acknowledge this interrupt to receive more interrupts from group 3
	//PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

	////
	// 	end normal interrupt code
	////

	DINT;
	PieCtrlRegs.PIEIER3.all = TempPIEIER3;
	//GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;
}

__interrupt void  EPwm5_timer_isr(void)
{
	//GpioDataRegs.GPASET.bit.GPIO0 = 1;
	// EPWM5 takes care of the time (in ms) as well as the analog reads (1KHz)
	increment_time_ms();
	// Clear INT flag for this timer
	EPwm5Regs.ETCLR.bit.INT = 1;
	// Acknowledge this interrupt to receive more interrupts from group 3
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
	//GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
}
