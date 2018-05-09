/*
 * SPI.h
 *
 *  Created on: Jun 30, 2016
 *      Author: zackl
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include "../HW/Interface.h"

#include "DSP2803x_Device.h"
#include "DSP2803x_Examples.h"

void spiCSN(void); 						// CS pin for external SPI low
void spiCSP(void); 						// CS pin for external SPI high
void spiSDN(void); 						// CS pin for SD card low
void spiSDP(void); 						// CS pin for SD card high
void spiInit(void); 					// Initialize SPI
void SPIIsrMap(void);
void SPIIsrEn(void);
void spi_MultiTx(uint16_t* dataOut, uint32_t count);
void spi_MultiRx(uint16_t* dataIn, uint16_t dataOut, uint32_t count);
uint16_t spi_Send(uint16_t data);

#endif /* SPI_H_ */
