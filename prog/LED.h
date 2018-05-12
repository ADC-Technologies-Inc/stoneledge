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

void LED_InitLeds(void);

void LED_Set(Uint16 req_);
void LED_Clear(Uint16 req_);

#endif /* PROG_LED_H_ */
