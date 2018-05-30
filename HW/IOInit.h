/*
 * IOInit.h
 *
 *  Created on: Apr 5, 2016
 *      Author: zlyzen
 */

#ifndef IOINIT_H_
#define IOINIT_H_

#include <stdint.h>
#include "../bits.h"

#include "I2C.h"
#include "../HW/ModuleConfig.h"
#include "DSP2803x_Device.h"     // DSP2803x Headerfile Include File

void InitGpio_start(void);

void ExtGpioInit(void); 											// initializes GPIO 100-807

// set an ext gpio pin high or low command (204,1) would set GPIO 204 high
void ExtGpioSet(uint16_t pin_, uint16_t set_); 						// set an ext GPIO pin high or low

// send a command (2,0x00) to set GPIO 200-207 all low if all are configured as outputs
void GpioSetDirArray(uint16_t pin_array_, uint16_t array_); 		// sets ext GPIO array high/low

// send a command such as 2 to get all pin states for GPIO 200-207
uint16_t ExtGpioGetSet(uint16_t pin_set_);

// send a request such as 206 to read the state of GPIO 206 and it will return a 1 or 0 depending on the state of the pin
uint16_t ExtGpioRead(uint16_t pin_);

#endif /* IOINIT_H_ */
