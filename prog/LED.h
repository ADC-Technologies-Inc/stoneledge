/*
 * LED.h
 *
 *  Created on: Mar 8, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_LED_H_
#define PROG_LED_H_

#include <stdint.h>
#include "../bits.h"

#include "../HW/IOInit.h"
#include "prog_conf.h"

void InitLeds(void);
void SetLed(Uint16 req_);
void ClearLed(Uint16 req_);
void LedWdSet(Uint16 Wd_);
void LedWdClear(Uint16 Wd_);

#endif /* PROG_LED_H_ */
