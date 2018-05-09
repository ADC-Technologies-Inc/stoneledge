/*
 * I2C.c
 *
 *  Created on: Aug 9, 2016
 *      Author: zackl
 *//*
#include "../HW/I2C.h"
#include "../prog/ntd_debug.h"

__interrupt void i2c_int1a_isr(void);
uint16_t rx_flag = 0;
uint16_t tx_flag = 0;

uint16_t two_count_total = 0;
uint16_t two_count_single = 0;

uint16_t one_count_total = 0;
uint16_t one_count_single = 0;

uint16_t rxinc = 0;
uint16_t txinc = 0;

uint16_t rxready = 0;
uint16_t rx_count = 0;

uint16_t txready = 0;
uint16_t tx_count = 0;

uint16_t i2c_fail = 0;

uint16_t * rx;
uint16_t * tx;

void  CheckI2CHold(void)
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
		GpioCtrlRegs.GPBPUD.bit.GPIO33 = 1;	   	// Enable pull-up for GPIO29 (SCLA)
		GpioDataRegs.GPBSET.bit.GPIO33 = 1;  	// set high
		GpioCtrlRegs.GPBDIR.bit.GPIO33 = 1;   	// output
		GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;   	// Configure GPIO29 for GPIO operation
		EDIS;
		// while data is low cycle clock

		//while(GpioDataRegs.GPBDAT.bit.GPIO32 == 0)
		while(j < 10)
		{
			i = TIME_MS;
			GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1;
			// delay for 5 ms
			while(i + 1 > TIME_MS)
			{
				asm(" NOP");
			}
			GpioDataRegs.GPBSET.bit.GPIO33 = 1;
			while(i + 2 > TIME_MS)
			{
				asm(" NOP");
			}
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
	GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;	   // Enable pull-up for GPIO29 (SCLA)
	GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // Asynch input GPIO29 (SCLA)
	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // Configure GPIO29 for SCLA operation
	EDIS;
}

void i2c_tx(uint16_t *buf_, uint16_t count_, uint16_t addr_)
{
	one_count_single=0;
	two_count_single=0;
	while(I2caRegs.I2CMDR.bit.STP == 1);

	do{
		i2c_fail = 0;
		CheckI2CHold();
		tx = buf_;
		txinc = 0;
		tx_flag = 1;
		I2caRegs.I2CCNT = count_;
		I2caRegs.I2CSAR = addr_;		// Slave address
		I2caRegs.I2CIER.bit.XRDY = 1;
		I2caRegs.I2CMDR.all = 0x6E20;

		//while(txready != 1);
		for(;;){
			if(!tx_flag) break;
		}
	}

	while(i2c_fail);
		return;
}

void i2c_rx(uint16_t * buf, uint16_t count, uint16_t loc, uint16_t addr_)
{
	one_count_single=0;
	two_count_single=0;
	while(I2caRegs.I2CMDR.bit.STP == 1 );			// wait for previous TX/RX to complete
	do{
		i2c_fail = 0;
		//I2caRegs.I2CMDR.all = 0;
		CheckI2CHold();
		rx = buf;
		rxinc = 0;
		rx_flag = 1;
		rxready = 0;
		// set slave address (NA in this code)

		I2caRegs.I2CIER.bit.ARDY = 1; 					// interrupt when ready to receive data
		I2caRegs.I2CCNT = 1; 							// data to send count (1 for device memory address)
		I2caRegs.I2CSAR = addr_;
		I2caRegs.I2CDXR = loc; 							// program in first address
		I2caRegs.I2CMDR.all = 0x2620;					// start condition, I2C master mode, TRX mode, enable I2C

		for(;;){
			if(rxready) break;
			}
		if(!i2c_fail)
		{
			I2caRegs.I2CCNT = count; 						// Setup how many bytes to expect
			I2caRegs.I2CIER.bit.RRDY = 1;
			I2caRegs.I2CMDR.all = 0x2C20;					// Send restart as master receiver stop bit
			//I2caRegs.I2CMDR.bit.STB

			for(;;){
				if(!rx_flag) break;
			}
		}
	} while(i2c_fail);
		return;
}

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

   I2caRegs.I2CPSC.all = 24;		// Prescaler - need 7-12 Mhz on module clk
   I2caRegs.I2CCLKL = 80;			// NOTE: must be non zero
   I2caRegs.I2CCLKH = 40;			// NOTE: must be non zero
   I2caRegs.I2CIER.all = 0x27;		// Enable AL, NACK, SCD, & ARDY interrupts

   I2caRegs.I2CMDR.all = 0x0420;	// Take I2C out of reset
   									// Stop I2C when suspended
   	   	   	   	   	   	   	   	   	// I2C master mode
   return;
}


__interrupt void i2c_int1a_isr(void)     // I2C-A
{
   Uint16 IntSource;

   // Read interrupt source
   IntSource = I2caRegs.I2CISRC.all;

   // stop condition detected
   if(IntSource == 6)
   {
	   if(I2caRegs.I2CSTR.all | 8)
		   rx[rxinc] = I2caRegs.I2CDRR;

	   I2caRegs.I2CIER.bit.RRDY = 0;
	   I2caRegs.I2CIER.bit.XRDY = 0;

	   if(rx_flag) rx_flag = 0;
	   if(tx_flag) tx_flag = 0;
   }

   // rx buffer ready
   if(IntSource == 4) 						// if read data ready
   {
	   rx[rxinc] = I2caRegs.I2CDRR;
	   rxinc++;
   }

   // tx buffer ready
   if(IntSource == 5) 						// if read data ready
   {
	   I2caRegs.I2CDXR = tx[txinc];
	   txinc++;
   }

   // register access ready
   if(IntSource == 3)
   {
	   if(rx_flag)
		   rxready = 1;
	   //I2caRegs.I2CIER.bit.ARDY = 0; 		// disable interrupt when ready to receive data
	   if(tx_flag)
	   {
		   I2caRegs.I2CDXR = tx[txinc];
		   txinc++;
		   txready = 1;
	   }
   }

   // NACK detected OR Arbitration lost
   if((IntSource == 2))// || (IntSource == 1))
   {
	   i2c_fail = 1; 						// retry i2c, failure detected
	   if(rx_flag && !rxready)
		   rxready = 1;
	   if(tx_flag && !txready)
		   txready = 1;
	   if(rx_flag) rx_flag = 0;
	   if(tx_flag) tx_flag = 0;
	   two_count_total++;
	   two_count_single++;
   }
   if(IntSource == 1)
   {
	   i2c_fail = 1; 						// retry i2c, failure detected
	   if(rx_flag && !rxready)
		   rxready = 1;
	   if(tx_flag && !txready)
		   txready = 1;
	   if(rx_flag) rx_flag = 0;
	   if(tx_flag) tx_flag = 0;
	   one_count_total++;
	   one_count_single++;
   }

   // Enable future I2C (PIE Group 8) interrupts
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP8;
}*/
