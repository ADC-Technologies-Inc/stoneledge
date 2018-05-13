/*
 * I2C.h
 *
 *  Created on: Aug 9, 2016
 *      Author: zackl
 */

#ifndef HW_I2C_H_
#define HW_I2C_H_

#include <stdint.h>

void I2C_Tx(uint16_t * buf_, uint16_t count_, uint16_t addr_);
void I2C_Rx(uint16_t * buf, uint16_t count, uint16_t loc, uint16_t addr_);

void I2C_ISRInit(void);
void I2C_ISREn(void);
void I2C_Init(void);

#endif /* HW_I2C_H_ */
