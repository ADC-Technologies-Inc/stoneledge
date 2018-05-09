/*
 * current.h
 *
 *  Created on: Mar 7, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_CURRENT_H_
#define PROG_CURRENT_H_

#include <stdint.h>
#include "../bits.h"

#include "prog_conf.h"
#include "../HW/Analog.h"

void ProcessCurrentSet(uint16_t set_);
uint32_t GetCurrentData(void);
uint32_t GetPowerData(void);
void InitCurrent(void);

#endif /* PROG_CURRENT_H_ */
