/*
 * I2C.c
 *
 *  Created on: Aug 9, 2016
 *      Author: zackl
 */
#include "../HW/I2C.h"
#include "../prog/ntd_debug.h"

#include "DSP2803x_Examples.h"


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


__interrupt void i2c_int1a_isr(void);
uint16_t rx_flag = 0;
uint16_t tx_flag = 0;
uint16_t fail_flag = 0;

uint16_t rxinc = 0;
uint16_t txinc = 0;

uint16_t rxready = 0;
uint16_t rx_count = 0;

uint16_t txready = 0;
uint16_t tx_count = 0;

uint16_t * rx;
uint16_t * tx;


void I2cIsrInit(void)
{
    EALLOW;
    PieVectTable.I2CINT1A = &i2c_int1a_isr;
    EDIS;
    return;
}

void I2cIsrEn(void)
{
    PieCtrlRegs.PIEIER8.bit.INTx1 = 1;
    IER |= M_INT8;
    return;
}

void I2cInit(void)
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
   I2caRegs.I2CPSC.bit.IPSC = 7;    // Prescaler - need 7-12 Mhz on module clk (SYSCLKOUT is set to 60MHz in FlashConfig.); 60/(7+1) = 7.5Mhz;
   I2caRegs.I2CCLKL = 20;           // NOTE: must be non zero,
   I2caRegs.I2CCLKH = 10;           // NOTE: must be non zero

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

   //Freedom! (etc)
   return;
}

void i2c_tx(uint16_t *buf_, uint16_t count_, uint16_t addr_)
{
//    printf("i2c_tx():: count= %d; addr_= %d\n", count_, addr_);
    int retry = 0;
    uint16_t tempIER = 0;

re_enter:

    DINT;       //disable group 3 interrupts before we hit the while loop, if we don't we have a potential re-entrancy issue

	while(I2caRegs.I2CSTR.bit.BB || tx_flag || rx_flag ); //Wait for bus and device to be available

	//Save PIE Group 3 status (make sure re-enter doesn't cause an issue)
	if (!tempIER && IER & 0x03){
	    tempIER = 0x03;
	    IER &= 0xFFFB;          //disable group 3 register
	}

	EINT;                       //re-enable interrupts

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

	for(;;){
	    if (!tx_flag) break;
	}

    if ( fail_flag ){
        if (retry==3){
            printf("i2c_tx():: Too many retries!\n");
            goto exit_;
        }
        retry++;
        goto re_enter;
    }

exit_:
    DINT;
    //Restore register
    if (tempIER){
        IER |= 0x03;
        IER &= 0x03;
    }
    EINT;

	//printf("i2c_tx():: I exited!\n");
	return;
}

void i2c_rx(uint16_t * buf, uint16_t count, uint16_t loc, uint16_t addr_)
{
    //printf("i2c_rx():: count= %d; addr_= %d\n", count, addr_);
    int retry = 0;
    uint16_t tempIER = 0;

re_enter:

    DINT;       //disable group 3 interrupts before we hit the while loop, if we don't we have a potential re-entrancy issue

    while(I2caRegs.I2CSTR.bit.BB || tx_flag || rx_flag ); //Wait for bus and device to be available

    //Save Group 3 status
    if (!tempIER && IER & 0x03){
        tempIER = 0x03;
        IER &= 0xFFFB;          //disable group 3 register
    }

    EINT;

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

	for(;;){
		if(rxready) break;
	}

    I2caRegs.I2CCNT = count; 						// Setup how many bytes to expect
    I2caRegs.I2CMDR.bit.TRX = 0;                    //set to receiver mode
    I2caRegs.I2CMDR.all |= I2CMDR_STT | I2CMDR_STP | I2CMDR_MST | I2CMDR_FREE; //set stop biy
    //I2caRegs.I2CMDR.all = 0x2C20;					// Send restart as master receiver stop bit
    //I2caRegs.I2CMDR.bit.STB

    for(;;){
        if (!rx_flag) break;
    }

    if ( fail_flag ){
        if (retry==3){
            printf("i2c_rx():: Too many retries!\n");
            goto exit_;
        }
        retry++;
        goto re_enter;
    }
exit_:
    DINT;
    //Restore register
    if (tempIER){
        IER |= 0x03;
        IER &= 0x03;
    }
    EINT;

    //printf("i2c_tx():: I exited too!\n");
    return;
}

__interrupt void i2c_int1a_isr(void)     // I2C-A
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

           printf("NACK\n");

           break;
   }

   // Enable future I2C (PIE Group 8) interrupts
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP8;
   return;
}
