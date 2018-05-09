/*
 * Analog.c
 *
 *  Created on: Apr 26, 2016
 *      Author: zlyzen
 */


//===========================================================================
// Includes
//===========================================================================
#include "../HW/Analog.h"

//===========================================================================
// Defines
//===========================================================================

#define SAMPLE_DEPTH 	10

//===========================================================================
// ISR Prototypes
//===========================================================================

__interrupt void adc_isr(void);

#pragma CODE_SECTION(adc_isr, "ramfuncs");

//===========================================================================
// Structs
//===========================================================================

struct Analog{
	uint16_t 	adcina0[SAMPLE_DEPTH];
	uint16_t 	adcina1[SAMPLE_DEPTH];
	uint16_t 	adcina2[SAMPLE_DEPTH];
	uint16_t 	adcina3[SAMPLE_DEPTH];
	uint16_t 	adcina4[SAMPLE_DEPTH];
	uint16_t 	adcina5[SAMPLE_DEPTH];
	uint16_t 	adcina6[SAMPLE_DEPTH];
	uint16_t 	adcina7[SAMPLE_DEPTH];
	uint16_t 	adcinb0[SAMPLE_DEPTH];
	uint16_t 	adcinb1[SAMPLE_DEPTH];
	uint16_t 	adcinb2[SAMPLE_DEPTH];
	//uint16_t 	adcinb3[SAMPLE_DEPTH];
	//uint16_t 	adcinb4[SAMPLE_DEPTH];
	//uint16_t 	adcinb5[SAMPLE_DEPTH];
	//uint16_t 	adcinb6[SAMPLE_DEPTH];
	uint16_t 	adcinb7[SAMPLE_DEPTH];
	uint16_t 	adcmux;
	uint16_t 	adcindex;
	uint16_t 	adcinten;
	uint16_t 	adcresready;
};

//===========================================================================
// Globals
//===========================================================================

static struct Analog AnalogChannels;			// holds measured info for motor current sense

//===========================================================================
// Hidden Function Prototypes
//===========================================================================


/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Running
 *
 -----------------------------------------------------------------------------------------------------------*/

void ProcessAnalogResult(void)
{
	if(AnalogChannels.adcresready == 1)
	{
		ProcessNtcSet(AnalogChannels.adcmux); 			// Average 12x10 samples that are ready and place into averaged PTC result array
		ProcessCurrentSet(AnalogChannels.adcmux);  		// Process current sense info
		if(AnalogChannels.adcmux == 15) 				// if this is the 16th ADC MUX channel then set PTC ready
		{
			SetNtcReady();
			AnalogChannels.adcmux = 0;
		}
		else
		{
			AnalogChannels.adcmux++;
		}

		SetMux(AnalogChannels.adcmux); 				// Change MUX channel
		// wait to re-enable analog reads after mux has been incremented
		// this will allow settling time before new reads start
		//EPwm5Regs.ETSEL.bit.SOCAEN = 1; 				// Enable SOC Trig (EPWM)
		AnalogChannels.adcresready = 0; 				// set results to not ready
		//AnalogChannels.adcinten = 1; 					// set interrupt to enabled
		AnalogChannels.adcindex = 0; 					// set index to 0
	}
}

void StartAnalog(void)
{
	EPwm5Regs.ETSEL.bit.SOCAEN = 1;
}

void StopAnalog(void)
{
	EPwm5Regs.ETSEL.bit.SOCAEN = 0;
}

/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Init
 *
 -----------------------------------------------------------------------------------------------------------*/
void InitializeAnalog(void)
{
	AnalogChannels.adcindex = 0;
	AnalogChannels.adcinten = 0;
	AnalogChannels.adcmux = 0;
	AnalogChannels.adcresready = 0;
}

uint16_t* GetAnalogAddress(uint16_t req_)
{
	switch (req_) {

	case 0 :
		return &AnalogChannels.adcina0[0];
	case 1 :
		return &AnalogChannels.adcina1[0];
	case 2 :
		return &AnalogChannels.adcina2[0];
	case 3 :
		return &AnalogChannels.adcina3[0];
	case 4 :
		return &AnalogChannels.adcina4[0];
	case 5 :
		return &AnalogChannels.adcina5[0];
	case 6 :
		return &AnalogChannels.adcina6[0];
	case 7 :
		return &AnalogChannels.adcina7[0];
	case 8 :
		return &AnalogChannels.adcinb0[0];
	case 9 :
		return &AnalogChannels.adcinb1[0];
	case 10 :
		return &AnalogChannels.adcinb2[0];
	case 11 :
		return &AnalogChannels.adcinb7[0];
	default :
		return 0;
	}
}

void ADCISREn(void)
{
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;					// Enable INT 1.1 in the PIE
    IER |= M_INT1; 										// Enable CPU Interrupt 1
}

void ADCISRMap(void)
{
	EALLOW;
    PieVectTable.ADCINT1 = &adc_isr;
    EDIS;
}

void ConfigADC()
{

// Configure ADC
// Note: Channel ADCINA4  will be double sampled to workaround the ADC 1st sample issue for rev0 silicon errata

   EALLOW;
   AdcRegs.ADCINTSOCSEL1.bit.SOC0  = 0;
   AdcRegs.ADCINTSOCSEL1.bit.SOC1  = 0;
   AdcRegs.ADCINTSOCSEL1.bit.SOC2  = 0;
   AdcRegs.ADCINTSOCSEL1.bit.SOC3  = 0;
   AdcRegs.ADCINTSOCSEL1.bit.SOC4  = 0;
   AdcRegs.ADCINTSOCSEL1.bit.SOC5  = 0;
   AdcRegs.ADCINTSOCSEL1.bit.SOC6  = 0;
   AdcRegs.ADCINTSOCSEL1.bit.SOC7  = 0;

   AdcRegs.ADCINTSOCSEL2.bit.SOC8  = 0;
   AdcRegs.ADCINTSOCSEL2.bit.SOC9  = 0;
   AdcRegs.ADCINTSOCSEL2.bit.SOC10 = 0;
   AdcRegs.ADCINTSOCSEL2.bit.SOC11 = 0;
   AdcRegs.ADCINTSOCSEL2.bit.SOC12 = 0;
   AdcRegs.ADCINTSOCSEL2.bit.SOC13 = 0;
   AdcRegs.ADCINTSOCSEL2.bit.SOC14 = 0;
   AdcRegs.ADCINTSOCSEL2.bit.SOC15 = 0;

   AdcRegs.ADCCTL1.bit.INTPULSEPOS	= 1;		// ADCINT1 trips after AdcResults latch
   AdcRegs.ADCCTL1.bit.ADCREFSEL 	= 0;

   AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;

   AdcRegs.INTSEL1N2.bit.INT1E      = 1;		// Enabled ADCINT1
   AdcRegs.INTSEL1N2.bit.INT1CONT   = 0;		// Disable ADCINT1 Continuous mode
   AdcRegs.INTSEL1N2.bit.INT1SEL	= 0x0C;		// setup EOC12 to trigger ADCINT1 to fire

   AdcRegs.ADCSOC0CTL.bit.CHSEL 	= 0x00;		// set SOC0 channel select to ADCINA0(dummy sample for rev0 errata workaround)

   AdcRegs.ADCSOC1CTL.bit.CHSEL 	= 0x00;		// set SOC1 channel select to ADCINA0
   AdcRegs.ADCSOC2CTL.bit.CHSEL 	= 0x01; 	// set SOC2 channel select to ADCINA1
   AdcRegs.ADCSOC3CTL.bit.CHSEL 	= 0x02; 	// set SOC3 channel select to ADCINA2
   AdcRegs.ADCSOC4CTL.bit.CHSEL 	= 0x03; 	// set SOC4 channel select to ADCINA3
   AdcRegs.ADCSOC5CTL.bit.CHSEL 	= 0x04; 	// set SOC5 channel select to ADCINA4
   AdcRegs.ADCSOC6CTL.bit.CHSEL 	= 0x05; 	// set SOC6 channel select to ADCINA5
   AdcRegs.ADCSOC7CTL.bit.CHSEL 	= 0x06; 	// set SOC7 channel select to ADCINA6
   AdcRegs.ADCSOC8CTL.bit.CHSEL 	= 0x07; 	// set SOC8 channel select to ADCINA7

   AdcRegs.ADCSOC9CTL.bit.CHSEL 	= 0x08;		// set SOC9  channel select to ADCINB0
   AdcRegs.ADCSOC10CTL.bit.CHSEL 	= 0x09; 	// set SOC10 channel select to ADCINB1
   AdcRegs.ADCSOC11CTL.bit.CHSEL 	= 0x0A; 	// set SOC11 channel select to ADCINB2
   //AdcRegs.ADCSOC12CTL.bit.CHSEL 	= 0x0B; 	// set SOC12 channel select to ADCINB3
   //AdcRegs.ADCSOC13CTL.bit.CHSEL 	= 0x0C; 	// set SOC13 channel select to ADCINB4
   //AdcRegs.ADCSOC14CTL.bit.CHSEL 	= 0x0D; 	// set SOC14 channel select to ADCINB5
   //AdcRegs.ADCSOC15CTL.bit.CHSEL 	= 0x0E; 	// set SOC15 channel select to ADCINB6
   AdcRegs.ADCSOC12CTL.bit.CHSEL 	= 0x0F; 	// set SOC13 channel select to ADCINB7

   AdcRegs.ADCSOC0CTL.bit.TRIGSEL 	= 0x0D;		//set SOC0 start trigger on EPWM1 SOCA, due to round-robin SOC0 converts first then SOC1, then SOC3
   AdcRegs.ADCSOC1CTL.bit.TRIGSEL 	= 0x0D;		//set SOC1 start trigger on EPWM1 SOCA, due to round-robin SOC0 converts first then SOC1, then SOC3
   AdcRegs.ADCSOC2CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC3CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC4CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC5CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC6CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC7CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC8CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC9CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC10CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC11CTL.bit.TRIGSEL 	= 0x0D;
   AdcRegs.ADCSOC12CTL.bit.TRIGSEL 	= 0x0D;

   AdcRegs.ADCSOC0CTL.bit.ACQPS 	= 6;		//set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
   AdcRegs.ADCSOC1CTL.bit.ACQPS 	= 21;		//set SOC1 S/H Window to 32 ADC Clock Cycles, (6 ACQPS plus  26)
   AdcRegs.ADCSOC2CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC3CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC4CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC5CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC6CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC7CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC8CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC9CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC10CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC11CTL.bit.ACQPS 	= 21;
   AdcRegs.ADCSOC12CTL.bit.ACQPS 	= 21;

   EDIS;

   return;

}

/*-----------------------------------------------------------------------------------------------------------
 *
 * ISRs
 *
 -----------------------------------------------------------------------------------------------------------*/
__interrupt void  adc_isr(void)
{//GpioDataRegs.GPASET.bit.GPIO2 = 1;
  AnalogChannels.adcina0[AnalogChannels.adcindex] = AdcResult.ADCRESULT1; //discard ADCRESULT0 as part of the workaround to the 1st sample errata for rev0
  AnalogChannels.adcina1[AnalogChannels.adcindex] = AdcResult.ADCRESULT2;
  AnalogChannels.adcina2[AnalogChannels.adcindex] = AdcResult.ADCRESULT3;
  AnalogChannels.adcina3[AnalogChannels.adcindex] = AdcResult.ADCRESULT4;
  AnalogChannels.adcina4[AnalogChannels.adcindex] = AdcResult.ADCRESULT5;
  AnalogChannels.adcina5[AnalogChannels.adcindex] = AdcResult.ADCRESULT6;
  AnalogChannels.adcina6[AnalogChannels.adcindex] = AdcResult.ADCRESULT7;
  AnalogChannels.adcina7[AnalogChannels.adcindex] = AdcResult.ADCRESULT8;

  AnalogChannels.adcinb0[AnalogChannels.adcindex] = AdcResult.ADCRESULT9;
  AnalogChannels.adcinb1[AnalogChannels.adcindex] = AdcResult.ADCRESULT10;
  AnalogChannels.adcinb2[AnalogChannels.adcindex] = AdcResult.ADCRESULT11;
  //AnalogChannels.adcinb3[AnalogChannels.adcindex] = AdcResult.ADCRESULT12;
  //AnalogChannels.adcinb4[AnalogChannels.adcindex] = AdcResult.ADCRESULT13;
  //AnalogChannels.adcinb5[AnalogChannels.adcindex] = AdcResult.ADCRESULT14;
  //AnalogChannels.adcinb6[AnalogChannels.adcindex] = AdcResult.ADCRESULT15;
  AnalogChannels.adcinb7[AnalogChannels.adcindex] = AdcResult.ADCRESULT12; // board current measurement - always - irrelevant of the ADC MUX

  AnalogChannels.adcindex++;

  if(AnalogChannels.adcindex >= SAMPLE_DEPTH)
  {
	  AnalogChannels.adcindex 			= 0; 				// Reset sample index
	  AnalogChannels.adcinten 			= 0; 				// Say that ADC interrupt is disabled
	  AnalogChannels.adcresready 		= 1; 				// ADC samples ready
	  EPwm5Regs.ETSEL.bit.SOCAEN        = 0; 				// Disable SOC Trig (EPWM) - stops reads from being triggered

  }

  AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;		//Clear ADCINT1 flag reinitialize for next SOC
  EPwm5Regs.ETCLR.bit.SOCA = 1;

  PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE
  //GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;
}
