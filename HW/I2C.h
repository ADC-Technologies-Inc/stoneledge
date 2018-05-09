/*
 * I2C.h
 *
 *  Created on: Aug 9, 2016
 *      Author: zackl
 */

#ifndef HW_I2C_H_
#define HW_I2C_H_

#include <stdint.h>

#include "DSP28x_Project.h"
#include "../prog/time.h"

void  CheckI2CHold(void);
void i2c_tx(uint16_t * buf_, uint16_t count_, uint16_t addr_);
void i2c_rx(uint16_t * buf, uint16_t count, uint16_t loc, uint16_t addr_);

void I2cIsrInit(void);
void I2cIsrEn(void);
void I2cInit(void);

#endif /* HW_I2C_H_ */
