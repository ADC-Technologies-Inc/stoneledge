
#include "../HW/hal.h"

#include "DSP2803x_Device.h"
#include "DSP2803x_PieCtrl.h"
#include "DSP2803x_GlobalPrototypes.h"

#include "../HW/ADC.h"
#include "../HW/Analog.h"
#include "../HW/I2C.h"
#include "../HW/Interface.h"
#include "../HW/IOInit.h"
#include "../HW/PWM.h"
#include "../HW/RealTimeClock.h"
#include "../HW/SPI.h"

//  *IMPORTANT*
//  IF RUNNING FROM FLASH, PLEASE COPY OVER THE SECTION "ramfuncs"  FROM FLASH
//  TO RAM PRIOR TO CALLING InitSysCtrl(). THIS PREVENTS THE MCU FROM THROWING
//  AN EXCEPTION WHEN A CALL TO DELAY_US() IS MADE.
//
#pragma CODE_SECTION(InitFlash, "ramfuncs");

void DisableGlobalInterrupts() {
	DINT;
	DRTM;
}


void EnableGlobalInterrupts() {
	EINT;   // Enable Global interrupt INTM
	ERTM;   // Enable Global realtime interrupt DBGM
}


void ServiceDog(void)
{
    EALLOW;
    SysCtrlRegs.WDKEY = 0x0055;
    SysCtrlRegs.WDKEY = 0x00AA;
    EDIS;
}

//---------------------------------------------------------------------------
// Example: DisableDog:
//---------------------------------------------------------------------------
// This function disables the watchdog timer.

void DisableDog(void)
{
    EALLOW;
    SysCtrlRegs.WDCR= 0x0068;
    EDIS;
}

void InitPll(Uint16 val, Uint16 divsel)
{
   volatile Uint16 iVol;

   // Make sure the PLL is not running in limp mode
   if (SysCtrlRegs.PLLSTS.bit.MCLKSTS != 0)
   {
      EALLOW;
      // OSCCLKSRC1 failure detected. PLL running in limp mode.
      // Re-enable missing clock logic.
      SysCtrlRegs.PLLSTS.bit.MCLKCLR = 1;
      EDIS;
      // Replace this line with a call to an appropriate
      // SystemShutdown(); function.
      __asm("        ESTOP0");     // Uncomment for debugging purposes
   }

   // DIVSEL MUST be 0 before PLLCR can be changed from
   // 0x0000. It is set to 0 by an external reset XRSn
   // This puts us in 1/4
   if (SysCtrlRegs.PLLSTS.bit.DIVSEL != 0)
   {
       EALLOW;
       SysCtrlRegs.PLLSTS.bit.DIVSEL = 0;
       EDIS;
   }

   // Change the PLLCR
   if (SysCtrlRegs.PLLCR.bit.DIV != val)
   {

      EALLOW;
      // Before setting PLLCR turn off missing clock detect logic
      SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1;
      SysCtrlRegs.PLLCR.bit.DIV = val;
      EDIS;

      // Optional: Wait for PLL to lock.
      // During this time the CPU will switch to OSCCLK/2 until
      // the PLL is stable.  Once the PLL is stable the CPU will
      // switch to the new PLL value.
      //
      // This time-to-lock is monitored by a PLL lock counter.
      //
      // Code is not required to sit and wait for the PLL to lock.
      // However, if the code does anything that is timing critical,
      // and requires the correct clock be locked, then it is best to
      // wait until this switching has completed.

      // Wait for the PLL lock bit to be set.

      // The watchdog should be disabled before this loop, or fed within
      // the loop via ServiceDog().

      // Uncomment to disable the watchdog
      DisableDog();

      while(SysCtrlRegs.PLLSTS.bit.PLLLOCKS != 1)
      {
          // Uncomment to service the watchdog
          // ServiceDog();
      }

      EALLOW;
      SysCtrlRegs.PLLSTS.bit.MCLKOFF = 0;
      EDIS;
    }

    // If switching to 1/2
    if((divsel == 1)||(divsel == 2))
    {
        EALLOW;
        SysCtrlRegs.PLLSTS.bit.DIVSEL = divsel;
        EDIS;
    }

    // If switching to 1/1
    // * First go to 1/2 and let the power settle
    //   The time required will depend on the system, this is only an example
    // * Then switch to 1/1
    if(divsel == 3)
    {
        EALLOW;
        SysCtrlRegs.PLLSTS.bit.DIVSEL = 2;
//        DELAY_US(50L);
        SysCtrlRegs.PLLSTS.bit.DIVSEL = 3;
        EDIS;
    }
}


#define CPU_RATE   16.667L   // for a 60MHz CPU clock speed (SYSCLKOUT)
#define Device_cal (void   (*)(void))0x3D7C80


void InitPeripheralClocks(void)
{
   EALLOW;

// LOSPCP prescale register settings, normally it will be set to default values

   //GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 3;  // GPIO18 = XCLKOUT
   SysCtrlRegs.LOSPCP.all = 0x0001; 		// peripherial clock = 60MHz / 2 (30MHz)

// XCLKOUT to SYSCLKOUT ratio.  By default XCLKOUT = 1/4 SYSCLKOUT
   SysCtrlRegs.XCLK.bit.XCLKOUTDIV=2;

// Peripheral clock enables set for the selected peripherals.
// If you are not using a peripheral leave the clock off
// to save on power.
//
// Note: not all peripherals are available on all 2803x derivates.
// Refer to the datasheet for your particular device.
//
// This function is not written to be an example of efficient code.

   SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;      // ADC
   SysCtrlRegs.PCLKCR3.bit.COMP1ENCLK = 1;    // COMP1
   SysCtrlRegs.PCLKCR3.bit.COMP2ENCLK = 1;    // COMP2
   SysCtrlRegs.PCLKCR3.bit.COMP3ENCLK = 1;    // COMP3
   SysCtrlRegs.PCLKCR1.bit.ECAP1ENCLK = 1;    // eCAP1
   SysCtrlRegs.PCLKCR0.bit.ECANAENCLK=1;      // eCAN-A
   SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;    // eQEP1
   SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;    // ePWM1
   SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1;    // ePWM2
   SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;    // ePWM3
   SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;    // ePWM4
   SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 1;    // ePWM5
   SysCtrlRegs.PCLKCR1.bit.EPWM6ENCLK = 1;    // ePWM6
   SysCtrlRegs.PCLKCR1.bit.EPWM7ENCLK = 1;    // ePWM7
   SysCtrlRegs.PCLKCR0.bit.HRPWMENCLK = 1;    // HRPWM
   SysCtrlRegs.PCLKCR0.bit.I2CAENCLK = 1;     // I2C
   SysCtrlRegs.PCLKCR0.bit.LINAENCLK = 1;     // LIN-A
   SysCtrlRegs.PCLKCR3.bit.CLA1ENCLK = 1;     // CLA1
   SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;     // SCI-A
   SysCtrlRegs.PCLKCR0.bit.SPIAENCLK = 1;     // SPI-A
   SysCtrlRegs.PCLKCR0.bit.SPIBENCLK = 1;     // SPI-B
   SysCtrlRegs.PCLKCR2.bit.HRCAP1ENCLK = 1;
   SysCtrlRegs.PCLKCR2.bit.HRCAP2ENCLK = 1;

   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;     // Enable TBCLK within the ePWM

   EDIS;
}

void IntOsc1Sel (void) {
    EALLOW;
    SysCtrlRegs.CLKCTL.bit.INTOSC1OFF = 0;
    SysCtrlRegs.CLKCTL.bit.OSCCLKSRCSEL=0;  // Clk Src = INTOSC1
    SysCtrlRegs.CLKCTL.bit.XCLKINOFF=1;     // Turn off XCLKIN
    SysCtrlRegs.CLKCTL.bit.XTALOSCOFF=1;    // Turn off XTALOSC
    SysCtrlRegs.CLKCTL.bit.INTOSC2OFF=1;    // Turn off INTOSC2
    EDIS;
}

void InitSysCtrl(void)
{

   // Disable the watchdog
   DisableDog();

   // *IMPORTANT*
   // The Device_cal function, which copies the ADC & oscillator calibration values
   // from TI reserved OTP into the appropriate trim registers, occurs automatically
   // in the Boot ROM. If the boot ROM code is bypassed during the debug process, the
   // following function MUST be called for the ADC and oscillators to function according
   // to specification. The clocks to the ADC MUST be enabled before calling this
   // function.
   // See the device data manual and/or the ADC Reference
   // Manual for more information.

        EALLOW;
        SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1; // Enable ADC peripheral clock
        (*Device_cal)();
        SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 0; // Return ADC clock to original state
        EDIS;

   // Select Internal Oscillator 1 as Clock Source (default), and turn off all unused clocks to
   // conserve power.
   IntOsc1Sel();

   // Initialize the PLL control: PLLCR and CLKINDIV
   // DSP28_PLLCR and DSP28_CLKINDIV are defined in DSP2803x_Examples.h
   InitPll(12,2);
   // Initialize the peripheral clocks
   InitPeripheralClocks();
}

//---------------------------------------------------------------------------
// Example: InitFlash:
//---------------------------------------------------------------------------
// This function initializes the Flash Control registers

//                   CAUTION
// This function MUST be executed out of RAM. Executing it
// out of OTP/Flash will yield unpredictable results

void InitFlash(void)
{
   EALLOW;
   //Enable Flash Pipeline mode to improve performance
   //of code executed from Flash.
   FlashRegs.FOPT.bit.ENPIPE = 1;

   //                CAUTION
   //Minimum waitstates required for the flash operating
   //at a given CPU rate must be characterized by TI.
   //Refer to the datasheet for the latest information.

   //Set the Paged Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 2;

   //Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 2;

   //Set the Waitstate for the OTP
   FlashRegs.FOTPWAIT.bit.OTPWAIT = 3;

   //                CAUTION
   //ONLY THE DEFAULT VALUE FOR THESE 2 REGISTERS SHOULD BE USED
   FlashRegs.FSTDBYWAIT.bit.STDBYWAIT = 0x01FF;
   FlashRegs.FACTIVEWAIT.bit.ACTIVEWAIT = 0x01FF;
   EDIS;

   //Force a pipeline flush to ensure that the write to
   //the last register configured occurs before returning.

   __asm(" RPT #7 || NOP");
}

void InitializeHardware(void)
{
	InitSysCtrl();

	InitEPwm();
	InitGpio_start();
	I2cInit();

	DINT;
	InitPieCtrl();
	IER = 0x0000;
	IFR = 0x0000;
	InitPieVectTable();

	InitFlash(); 							// initializes flash memory

	ADCISRMap();							// maps ADC ISR to Analog.c file
	SPIIsrMap(); 							// maps SPI ISR to SPI.c file
	I2cIsrInit(); 							// maps I2C ISR to I2C.c file
	PWMISRMap();

    InitAdc();  							// Init the ADC
    InitAdcAio(); 							//
    AdcOffsetSelfCal();						//
    ConfigADC();							// Sets up ADC triggering and other parameters

    ADCISREn();
    SPIIsrEn();
    I2cIsrEn();
    PWMISREn();

	InitializeRealTimeClocks();

	spiInit(); 								// initialize SPI perip.
	InitializeAnalog(); 	 				// initializes ADC struct
    EALLOW;
    // TODO enable WDT
    SysCtrlRegs.WDCR= 0x0068;
    EDIS;

    EnableGlobalInterrupts();


}


