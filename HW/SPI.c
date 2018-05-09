/*
 * SPI.c
 *
 *  Created on: Jun 30, 2016
 *      Author: zackl
 */
#include "../HW/SPI.h"

__interrupt void spiTxFifoIsr(void);
__interrupt void spiRxFifoIsr(void);

#pragma CODE_SECTION(spiTxFifoIsr, "ramfuncs");
#pragma CODE_SECTION(spiTxFifoIsr, "ramfuncs");

static uint16_t rxcount = 0;
static uint16_t txcount = 0;

static uint16_t * txbuf;
static uint16_t * rxbuf;

static uint16_t rxDump;

static uint16_t txbyte;
static uint16_t rxbyte;



void spiCSN(void){
	// GPIO19 low
	GpioDataRegs.GPACLEAR.bit.GPIO19 	= 1;
	return;
}

void spiCSP(void){
	// GPIO19 high
	GpioDataRegs.GPASET.bit.GPIO19 		= 1;
	return;
}

void spiSDN(void){
	// GPIO19 low
	GpioDataRegs.GPBCLEAR.bit.GPIO38 	= 1;
	return;
}

void spiSDP(void){
	// GPIO19 high
	GpioDataRegs.GPBSET.bit.GPIO38 		= 1;
	return;
}

void spiInit(void)
{

	spiCSP(); 											// SPI CSN pin high
	spiSDP(); 											// SPI CSN pin high (SD card)
	// Initialize SPI registers (FIFO)
	SpiaRegs.SPICCR.bit.SPISWRESET 	=0; 				// Reset SPI

	SpiaRegs.SPICCR.all		=0x0007;       				// 8-bit character length
	SpiaRegs.SPICTL.all		=0x001F;       				// Interrupt enabled, Master/Slave XMIT enabled
	SpiaRegs.SPISTS.all		=0x0000;					// SPI STATUS REGISTER RESET
	SpiaRegs.SPIBRR 		=0x03;           			// Baud rate LSPCLK/4 (MAX SPI SPEED)
	//SpiaRegs.SPIBRR 		=0x10;           			// Baud rate LSPCLK/4 (MAX SPI SPEED)
	SpiaRegs.SPIFFTX.all 	=0xC041;      				// Enable FIFO's, set TX FIFO level to 4
	SpiaRegs.SPIFFRX.all 	=0x0041;      				// Set RX FIFO level to 4
	SpiaRegs.SPIFFCT.all 	=0x00;						// SPI FIFO CONTROL REGISTER SETTINGS
	SpiaRegs.SPIPRI.all		=0x0010;					// SPI PRIORITY REGISTER SETTINGS

	SpiaRegs.SPICCR.bit.SPISWRESET	=1; 				// Enable SPI

	SpiaRegs.SPIFFTX.bit.TXFIFO		=1;					// Enable TX FIFO
	SpiaRegs.SPIFFRX.bit.RXFIFORESET	=1;				// Enable RX FIFO
	//rxbyte = SpiaRegs.SPIRXBUF;

}

void SPIIsrMap(void)
{
	   EALLOW;
	   PieVectTable.SPIRXINTA = &spiRxFifoIsr;
	   PieVectTable.SPITXINTA = &spiTxFifoIsr;
	   EDIS;
	   return;
}

void SPIIsrEn(void)
{
	   //PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   	// Enable the PIE block
	   PieCtrlRegs.PIEIER6.bit.INTx1=1;     	// Enable PIE Group 6, INT 1
	   PieCtrlRegs.PIEIER6.bit.INTx2=1;    	 	// Enable PIE Group 6, INT 2
	   IER |= M_INT6; 							// Enable CPU INT6

}

/**
 * @brief  Sends single byte over SPI
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  data: 8-bit data size to send over SPI
 * @retval Received byte from slave device
 */
/*static__INLINE*/ uint16_t spi_Send(uint16_t data)
{
	//spiSDN();
	//rxbuf = &rxbyte; 								// data RX'd from SPI interrupt is stored in rxbuf's destination
													// set it to rxbyte (which is returned by this function)
	txbyte = data;									// this is the single byte to send (usually the "dummy byte" in multi RX)
	rxcount = 1; 									// one byte to send
	txcount = 0;									// always start this at 0 for RX
	//SpiaRegs.SPIFFRX.bit.RXFIFORESET	=0;			// Disable RX FIFO
	//SpiaRegs.SPIFFRX.bit.RXFIFORESET	=1;			// Enable RX FIFO
	SpiaRegs.SPITXBUF = txbyte << 8; 				// send the first byte (to start the RX interrupt)
	//SpiaRegs.SPIFFTX.bit.TXFIFO = 1;

	//while(rxcount)								// while there are bytes left to rx
	//{
													// do nothing while rx'ing
	//}

	while(SpiaRegs.SPIFFRX.bit.RXFFST == 0) { }

	//SpiaRegs.SPIFFRX.bit.RXFIFORESET	=0;			// Disable RX FIFO
	rxbyte = SpiaRegs.SPIRXBUF;// >> 8;
	return rxbyte;

}

void spi_MultiTx(uint16_t* dataOut, uint32_t count)
{
	txcount = count;
	rxcount = 0;
	txbuf = dataOut;
	//SpiaRegs.SPIFFTX.bit.TXFIFO		=1;				// Enable TX FIFO
	SpiaRegs.SPIFFTX.all	|= BIT5;       				// Interrupt enabled

	//while(txcount > 0);									// while there are bytes left to tx

	for(;;){
		if(txcount == 0) break;
	}
													// do nothing while sending


	//SpiaRegs.SPIFFTX.bit.TXFIFO		=0;				// Enable TX FIFO
	SpiaRegs.SPIFFTX.all	&= ~BIT5;       				// Interrupt disabled, Master/Slave XMIT enabled
	// delay wait for tx to complete
	//delay_loop();
	//delay_loop();
	//delay_loop();
	while(SpiaRegs.SPIFFRX.bit.RXFFST) 				// flush rx buffer
	{
		rxDump = SpiaRegs.SPIRXBUF; 		 	// put the RX'd data into the RX buffer
	}
	return;
}

void spi_MultiRx(uint16_t* dataIn, uint16_t dataOut, uint32_t count)
{
	//SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1; 					// Clear Interrupt flag

	rxcount = count;
	txcount = 0;
	rxbuf = dataIn;
	txbyte = dataOut;

	/*
	while(SpiaRegs.SPIFFRX.bit.RXFFST)
	{
		rxbuf[txcount] = SpiaRegs.SPIRXBUF; 		 	// put the RX'd data into the RX buffer
	}
	*/
    SpiaRegs.SPIFFRX.bit.RXFFOVFCLR=1; 					// Clear Overflow flag
    SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1; 					// Clear Interrupt flag

	SpiaRegs.SPIFFRX.all	|= BIT5;
	//SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1; 					// Clear Interrupt flag
	SpiaRegs.SPITXBUF = txbyte << 8;

	//while(rxcount != 0);									// while there are bytes left to rx

		for(;;){
			if(rxcount == 0) break;// do nothing while rx'ing
		}


	//SpiaRegs.SPIFFRX.bit.RXFIFORESET	=0;			// Disable RX FIFO
	SpiaRegs.SPIFFRX.all		&= ~BIT5;
	// delay wait for tx to complete
	return;
}

//===========================================================================
// SPI ISRs
//===========================================================================
__interrupt void spiTxFifoIsr(void)
{
	if(txcount > 0) 										// if there is data left to TX
	{
		SpiaRegs.SPITXBUF = txbuf[rxcount] << 8; 			// add output data from output buffer to TX buffer
		txcount --; 									// decrement TX count
		rxcount ++; 									// increment RX count (used for moving through the TX buffer)
	}
	else 												// if all of the data bytes have been sent
	{
		//SpiaRegs.SPIFFTX.bit.TXFIFO	= 0;				// Disable TX FIFO
		SpiaRegs.SPIFFTX.bit.TXFFIENA = 0;
	}
    SpiaRegs.SPIFFTX.bit.TXFFINTCLR = 1; 				// Clear Interrupt flag
    PieCtrlRegs.PIEACK.all=0x20;       				// Issue PIE ACK
}

__interrupt void spiRxFifoIsr(void)
{
	if(rxcount > 0) 										// if there are bytes left we would like to read
	{
		rxbuf[txcount] = SpiaRegs.SPIRXBUF; 		 	// put the RX'd data into the RX buffer
		rxcount --; 									// decrement RX count
		txcount ++; 									// increment TX count (used for moving through the RX buffer)
		/*if(rxcount > 0)*/ SpiaRegs.SPITXBUF = txbyte << 8; 	// if there is more to RX
	}
	else 												// if all bytes have been received
	{
		SpiaRegs.SPIFFRX.bit.RXFFIENA = 0;
	}
    SpiaRegs.SPIFFRX.bit.RXFFOVFCLR=1; 					// Clear Overflow flag
    SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1; 					// Clear Interrupt flag
    PieCtrlRegs.PIEACK.all=0x20; 						// Issue PIE ackz
}
