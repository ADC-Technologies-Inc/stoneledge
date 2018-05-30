/*
 * I2C.c
 *
 *  Created on: Aug 9, 2016
 *      Author: zackl
 */
#include "../HW/I2C.h"

#include "../prog/ntd_debug.h"
#include "DSP28x_Project.h"
#include "DSP2803x_Examples.h"
#include "../prog/time.h"
#include "../HW/I2C.h"
#include "../HW/IOInit.h"
#include "../HW/Analog.h"

#define I2CMDR_NACKMOD  0x8000
#define I2CMDR_FREE     0x4000
#define I2CMDR_STT      0x2000
#define I2CMDR_STP      0x0800
#define I2CMDR_MST      0x0400
#define I2CMDR_TRX      0x0200
#define I2CMDR_XA       0x0100
#define I2CMDR_RM       0x0080
#define I2CMDR_DLB      0x0040
#define I2CMDR_IRS      0x0020
#define I2CMDR_STB      0x0010
#define I2CMDR_FDF      0x0008

void CheckI2CHold(void);
void I2C_ResetBus(void);
__interrupt void I2C_ISR(void);

static uint16_t rx_flag = 0;
static uint16_t tx_flag = 0;
static uint16_t fail_flag = 0;

static uint16_t rxinc = 0;
static uint16_t txinc = 0;

static uint16_t rxready = 0;
static uint16_t rx_count = 0;

static uint16_t txready = 0;
static uint16_t tx_count = 0;

static uint16_t * rx;
static uint16_t * tx;

//tracker to allow the I2C_ResetBus() to determine if a hard reset is necessary, reset to 0 on every successful tx/rx
static int reinit_attempts = 0;

void I2C_ISRInit(void)
{
    EALLOW;
    PieVectTable.I2CINT1A = &I2C_ISR;
    EDIS;
    return;
}

void I2C_ISREn(void)
{
    PieCtrlRegs.PIEIER8.bit.INTx1 = 1;
    IER |= M_INT8;
    return;
}

void I2C_Init(void)
{
   // Initialize I2C

    // set gpio for data to i2c
   // set gpio for clock to i2c
   EALLOW;
   // GPIO32 - SDA
   GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;    // Enable pull-up for GPIO28 (SDAA)
   GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  // Asynch input GPIO28 (SDAA)
   GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;   // Configure GPIO28 for SDAA operation

   // GPIO33 - SCL
   GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;    // Enable pull-up for GPIO29 (SCLA)
   GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // Asynch input GPIO29 (SCLA)
   GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // Configure GPIO29 for SCLA operation
   EDIS;

   I2caRegs.I2CMDR.bit.IRS = 0;     // Reset I2C

   //Clocks set to be within spec and target ~100kHz; (7,8,4 ~= 95kHz), (7, 20, 10) ~= 50kHz

   /*
    *   page 14, SPRUFZ9D.pdf
    *   Module clock [Tmod] = (I2C Input Freq) / (IPSC + 1)
    */
   I2caRegs.I2CPSC.bit.IPSC = 7;    // Prescaler - need 7-12 Mhz on module clk (SYSCLKOUT is set to 60MHz in FlashConfig.); 60/(7+1) = 7.5Mhz;

   /*
    *   Master clock (bus clock) = ((IPSC + 1) * [(ICCL +5) + (ICCH + 5)]) / [I2C Input Freq.]
    *
        IPSC    ICCL    ICCH        Module Clock    Master Clock Period Master Clock Freq
        7   20  10      7,500,000   5.33E-06        187,500
        7   20  20      7,500,000   6.67E-06        150,000
        7   30  30      7,500,000   9.33E-06        107,143
        7   40  40      7,500,000   1.20E-05        83,333
        7   50  50      7,500,000   1.47E-05        68,182
        7   100 100     7,500,000   2.80E-05        35,714
        7   150 150     7,500,000   4.13E-05        24,194
        7   5   5       7,500,000   2.67E-06        375,000
        7   5   10      7,500,000   3.33E-06        300,000
        7   5   15      7,500,000   4.00E-06        250,000
        7   10  10      7,500,000   4.00E-06        250,000


    *
    *
    */
   I2caRegs.I2CCLKL = 5;           // NOTE: must be non zero,
   I2caRegs.I2CCLKH = 5;           // NOTE: must be non zero

   I2caRegs.I2CIER.all = 0x00;
   I2caRegs.I2CIER.bit.SCD = 1;     //Stop Condition
   I2caRegs.I2CIER.bit.ARDY = 1;    //Register Access Ready
   I2caRegs.I2CIER.bit.XRDY = 1;       //Enable XMIT Ready Interrupt
   I2caRegs.I2CIER.bit.RRDY = 1;       //Enable RCV Ready Interrupt
   I2caRegs.I2CIER.bit.NACK = 1;       //Enable NACK Interrupt

   //I2caRegs.I2CIER.all = 0x24;        // Enable SCD & ARDY interrupts
   // 0b0000000000001010
   // receive ready & nack

   I2caRegs.I2CMDR.all = I2CMDR_MST | I2CMDR_IRS | I2CMDR_FREE;

   //I2caRegs.I2CMDR.bit.MST = 1;     //Master mode
   //I2caRegs.I2CMDR.bit.IRS = 1;     //Let loose
   //I2caRegs.I2CMDR.bit.FREE = 1;    //Continue operating while breakpoints fire

   //I2caRegs.I2CMDR.all = 0x0420;  // Take I2C out of reset
                                    // Stop I2C when suspended
                                    // I2C master mode

   //Delay to bring the BB bit into good condition (see section 5.4 of SPRUFZ9D)
   DELAY_US(20);

   //reset
   reinit_attempts = 0;

   //Freedom! (etc)
   return;
}

/*void CheckI2CHold(void)
{
    static uint32_t i;
    static uint16_t j;
    j = 0;
    // set gpio for data to input
    // set gpio for clock to output
    EALLOW;
    // GPIO32 - SDA
    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 1;    // Enable pull-up for GPIO28 (SDAA)
    GpioCtrlRegs.GPBDIR.bit.GPIO32 = 0;    // input
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;   // Configure GPIO28 for GPIO operation
    EDIS;
    if(GpioDataRegs.GPBDAT.bit.GPIO32 == 0)
    {
        EALLOW;
        // GPIO33 - SCL
        GpioCtrlRegs.GPBPUD.bit.GPIO33 = 1;     // Enable pull-up for GPIO29 (SCLA)
        GpioDataRegs.GPBSET.bit.GPIO33 = 1;     // set high
        GpioCtrlRegs.GPBDIR.bit.GPIO33 = 1;     // output
        GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;    // Configure GPIO29 for GPIO operation
        EDIS;
        // while data is low cycle clock

        //while(GpioDataRegs.GPBDAT.bit.GPIO32 == 0)
        while(j < 10)
        {
            GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1;
            DELAY_US(5000);

            GpioDataRegs.GPBSET.bit.GPIO33 = 1;
            DELAY_US(5000);

            j++;
        }
    }

    // set gpio for data back to i2c
    // set gpio for clock back to i2c
    EALLOW;
    // GPIO32 - SDA
    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;    // Enable pull-up for GPIO28 (SDAA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  // Asynch input GPIO28 (SDAA)
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;   // Configure GPIO28 for SDAA operation

    // GPIO33 - SCL
    GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;    // Enable pull-up for GPIO29 (SCLA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // Asynch input GPIO29 (SCLA)
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // Configure GPIO29 for SCLA operation
    EDIS;
}*/

void I2C_ResetBus(void){
    static int doing_reset = 0;
    uint16_t i = 0;

    #ifdef DEBUG_I2C
    printf("I2C_ResetBus():: Attempting to clear bus\n");
    #endif

    //Do a hard reset
    if (reinit_attempts++ == 3){
        if (doing_reset){
            //this is bad..
            #ifdef DEBUG_I2C
            printf("I2C_ResetBus():: Called while already resetting bus, aborting to allow reset to continue - this will result in a hang condition, we should probably just reboot everything :o \n");
            #endif

            return;
        }

        #ifdef DEBUG_I2C
        printf("I2C_ResetBus():: Performing a hard reset of I2C\n");
        #endif

        //Let the ADC code know that the channels are unstable and to discard data
        AnalogDiscard(FLAG_START);

        doing_reset = 1;

        //Reset the extGPIO (on GPIO24)
        I2caRegs.I2CMDR.bit.IRS = 0;            // Bring I2C down first.

        //Reset our I2C lines to a default condition
        EALLOW;
        GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;    //SDA
        GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;    //SCL
        EDIS;

        //Set both SDA/SCL low
        GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1;
        GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1;

        //reset ext-GPIO
        GpioDataRegs.GPACLEAR.bit.GPIO24 = 1;

        //hold for at least 16 ns
        DELAY_US(1);

        //lift reset signal
        GpioDataRegs.GPASET.bit.GPIO24 = 1;

        //hold for at least 400ns
        DELAY_US(1);

        //re-initialize I2C
        I2C_Init();

        //re-initialize the extGPIO to the correct state
        ExtGpioInit();

        //Let the ADC code know that the channels should be stable now
        AnalogDiscard(FLAG_END);

        reinit_attempts = 0;
        doing_reset = 0;

        return;
    }

    #ifdef DEBUG_I2C
    printf("I2C_ResetBus():: Sending 9 clocks cycles down the line\n");
    #endif

    //Reset clock.data lines as GPIO
    #ifdef I2C_DEBUG
    printf("I2C_ResetBus():: Bus Reset\n");
    #endif

    EALLOW;
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;    //SDA
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;    //SCL
    EDIS;

    //Send 9 clocks down the line to reset
    GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1;

    for (i=0;i<18;i++){
        GpioDataRegs.GPBTOGGLE.bit.GPIO33 = 1;
        DELAY_US(1000);
    }

    EALLOW;
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;    //SDA
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;    //SCL
    EDIS;

    DELAY_US(20);
}

#ifdef DEBUG_I2C
#define I2C_WAITFOR(flag_, MSG_)\
    times_round = 0;\
    for(;;){\
        if (flag_) break;\
        if (times_round++ > 10000) {\
            printf(MSG_);\
            I2C_ResetBus();\
            retry++;\
            tx_flag=rx_flag=0;\
            goto re_enter;\
        }\
    }
#else
#define I2C_WAITFOR(flag_, MSG_)\
    times_round = 0;\
    for(;;){\
        if (flag_) break;\
        if (times_round++ > 10000) {\
            I2C_ResetBus();\
            tx_flag=rx_flag=0;\
            retry++;\
            goto re_enter;\
        }\
    }
#endif

void I2C_Tx(uint16_t *buf_, uint16_t count_, uint16_t addr_)
{
//    printf("i2c_tx():: count= %d; addr_= %d\n", count_, addr_);
    uint16_t times_round = 0;
    int retry = 0;
    uint16_t tempIER = 0;

    //CheckI2CHold();

re_enter:

    DINT;                           //disable interrupts before we hit the while loop, if we don't we have a potential re-entrancy issue

    ASSERT( !tx_flag && !rx_flag);  //neither of these should be possible as i2c has the highest priority due to the MUX

    I2C_WAITFOR((!I2caRegs.I2CSTR.bit.BB), "I2C_Tx():: Resetting bus waiting for bus\n");
	//while(I2caRegs.I2CSTR.bit.BB ); //Wait for bus and device to be available

	//Save PIE Group 3 status (make sure re-enter doesn't cause an issue)
	if (!tempIER && IER & 0x04){    //check if the setting has already been saved first and then if INT3 is set
	    tempIER = 0x04;             //save INT3 setting
	    IER &= 0xFFFB;              //disable group 3 register
	}
	EINT;                           //re-enable interrupts*/

	tx_flag = 1;
	fail_flag = 0;

	tx = buf_;
	txinc = 0;
	txready = 0;

	//Setup for addressing segment
	I2caRegs.I2CCNT = count_;
	I2caRegs.I2CSAR = addr_ & 0x7F;		// Slave address, 7bit maximum

	//Enable for XRDY [address gone] and NACK [unhappy bunny on the other end] interrupts in addition to the Stop and ARDY Inters.
	I2caRegs.I2CIER.bit.XRDY = 1;       //Enable XMIT Ready Interrupt
	I2caRegs.I2CIER.bit.NACK = 1;       //Enable NACK Interrupt
	I2caRegs.I2CIER.bit.SCD = 1;        //Stop Condition, set in init
	I2caRegs.I2CIER.bit.ARDY = 1;	    //Registers Ready, set in init

	//printf("i2c_tx():: I2CCNT: %d, I2CSAR: %d, I2CIER: %d!\n", I2caRegs.I2CCNT, I2caRegs.I2CSAR, I2caRegs.I2CIER);

	I2caRegs.I2CMDR.all |= I2CMDR_STT | I2CMDR_TRX | I2CMDR_STP | I2CMDR_MST | I2CMDR_FREE;
	//I2caRegs.I2CMDR.all = 0x6E20;

	I2C_WAITFOR(!(tx_flag), "I2C_Tx():: Resetting bus waiting for tx_flag\n");
	//for(;;){
	//    if (!tx_flag) break;
	//}
    /*times_round = 0;
    for(;;){
        if (!tx_flag) break;
        if (times_round++ > 1000) {
            printf("I2C_Tx():: Resetting bus waiting for tx_flag\n");
            I2C_ResetBus();
            retry++;
            tx_flag=rx_flag=0;
            goto re_enter;
        }
    }*/

    if ( fail_flag ){
        if (retry==3){
            #ifdef DEBUG_I2C
            printf("I2C_Tx():: Too many retries\n");
            #endif
            goto exit_;
        }
        retry++;
        goto re_enter;
    }

exit_:
    //Restore register
    DINT;
    IER |= tempIER;
    EINT;

    reinit_attempts = 0;

	//printf("i2c_tx():: I exited!\n");
	return;
}

void I2C_Rx(uint16_t * buf, uint16_t count, uint16_t loc, uint16_t addr_)
{
    //printf("i2c_rx():: count= %d; addr_= %d\n", count, addr_);
    int retry = 0;
    uint16_t times_round = 0;
    uint16_t tempIER = 0;

    //CheckI2CHold();

re_enter:

    DINT;       //disable group 3 interrupts before we hit the while loop, if we don't we have a potential re-entrancy issue

    ASSERT( !tx_flag && !rx_flag);
    I2C_WAITFOR((!I2caRegs.I2CSTR.bit.BB), "I2C_Rx():: Resetting bus waiting for bus\n");
    //while(I2caRegs.I2CSTR.bit.BB  ); //Wait for bus and device to be available

    //Save PIE Group 3 status (make sure re-enter doesn't cause an issue)
    if (!tempIER && IER & 0x04){    //check if the setting has already been saved first and then if INT3 is set
        tempIER = 0x04;             //save INT3 setting
        IER &= 0xFFFB;              //disable group 3 register
    }
    EINT;                           //re-enable interrupts*/

	rx_flag = 1;
	fail_flag = 0;
	rx = buf;
	rxinc = 0;
	rxready = 0;
	// set slave address (NA in this code)

	I2caRegs.I2CCNT = 1; 							// data to send count (1 for device memory address)
	I2caRegs.I2CSAR = (addr_ & 0x7F) | 0x80;
	I2caRegs.I2CDXR = loc; 							// program in first address

    I2caRegs.I2CMDR.all |= I2CMDR_STT | I2CMDR_TRX | I2CMDR_MST | I2CMDR_FREE;
	//I2caRegs.I2CMDR.all = 0x2620;					// start condition, I2C master mode, TRX mode, enable I2C

    /*times_round = 0;
    for(;;){
        if (rxready) break;
        if (times_round++ > 1000) {
            printf("I2C_Rx():: Resetting bus waiting for rxready\n");
            I2C_ResetBus();
            retry++;
            tx_flag=rx_flag=0;
            goto re_enter;
        }
    }*/
    I2C_WAITFOR(rxready, "I2C_Rx():: Resetting bus waiting for rxready\n");
	//for(;;){
	//	if(rxready) break;
	//}

    I2caRegs.I2CCNT = count; 						// Setup how many bytes to expect
    I2caRegs.I2CMDR.bit.TRX = 0;                    //set to receiver mode
    I2caRegs.I2CMDR.all |= I2CMDR_STT | I2CMDR_STP | I2CMDR_MST | I2CMDR_FREE; //set stop biy
    //I2caRegs.I2CMDR.all = 0x2C20;					// Send restart as master receiver stop bit
    //I2caRegs.I2CMDR.bit.STB

    /*times_round = 0;
    for(;;){
        if (rxready) break;
        if (times_round++ > 1000) {
            printf("Resetting bus waiting for rx_flag\n");
            I2C_ResetBus();
            retry++;
            tx_flag=rx_flag=0;
            goto re_enter;
        }
    }*/
    I2C_WAITFOR( (!rx_flag) , "I2C_Rx():: Resetting bus waiting for rx_flag\n");
    /*for(;;){
        if (!rx_flag) break;
    }*/

    if ( fail_flag ){
        if (retry==3){
            #ifdef I2C_DEBUG
            printf("I2C_Rx():: Too many retries\n");
            #endif

            goto exit_;
        }
        retry++;
        goto re_enter;
    }
exit_:
    DINT;
    IER |= tempIER;
    EINT;

    reinit_attempts = 0;

    //printf("i2c_tx():: I exited too!\n");
    return;
}

__interrupt void I2C_ISR(void)     // I2C-A
{
   Uint16 IntSource;

   // Read interrupt source
   IntSource = I2caRegs.I2CISRC.bit.INTCODE;

   //printf("i2c_int1a_isr():: IntSource=%d\n", IntSource);

   switch(IntSource){
       case 6: //STOP DETECTED
		   if(rx_flag && I2caRegs.I2CSTR.bit.RRDY) //Check if there's a receive bit in here.
			   rx[rxinc++] = I2caRegs.I2CDRR;

		   //Reset status register (won't otherwise)
		   I2caRegs.I2CSTR.bit.RRDY = 1;
		   I2caRegs.I2CSTR.bit.XRDY = 1;
		   I2caRegs.I2CSTR.bit.SCD = 1;
		   I2caRegs.I2CSTR.bit.ARDY = 1;

		   //Reset flags, only one should be flagged at this point so reset both is ok.
		   rx_flag = 0;
		   tx_flag = 0;
		   break;
       case 4: //RX DATA READY
           if (!rx_flag)break;//noop
		   rx[rxinc++] = I2caRegs.I2CDRR;
		   break;
       case 5: //TRANSMIT DATA READY
           if (!tx_flag)break;//noop
		   I2caRegs.I2CDXR = tx[txinc++];
		   break;
       case 3: //REGISTERS READY - for TX, step 1: is to put the address on the bus (above), after that comes step 2: put the data on the bus!
           // register access ready
		   if(rx_flag){
		       //Address and request gone, start picking up data
			   rxready = 1;
			   break;
		   }

		   if(tx_flag){
		       //Put the first transmit data on the bus
			   I2caRegs.I2CDXR = tx[txinc];
			   txinc++;

			   //Ready to roll
			   txready = 1;
		   }
		   break;
       case 2: //NACK
           //Try again?
           fail_flag = 1;
           if (rx_flag) rx_flag = 0;
           if (tx_flag) tx_flag = 0;

           #ifdef I2C_DEBUG
           printf("I2C_ISR():: NACK\n");
           #endif

           break;
   }

   // Enable future I2C (PIE Group 8) interrupts
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP8;
   return;
}
