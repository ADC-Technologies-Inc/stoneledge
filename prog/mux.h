/*
 * mux.h
 *
 *  Created on: Mar 9, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_MUX_H_
#define PROG_MUX_H_

#include <stdint.h>
#include "../bits.h"

#include "../HW/IOInit.h"

void InitMux(void);
uint16_t GetMux(void);
void SetMux(uint16_t channel_);


#endif /* PROG_MUX_H_ */
