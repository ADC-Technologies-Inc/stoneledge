/*
 * duty_sw.h
 *
 *  Created on: 10 May 2018
 *      Author: ntd
 */

#ifndef PROG_DUTY_SW_H_
#define PROG_DUTY_SW_H_

#include <stdint.h>

#include "prog_conf.h"
#include "../HW/IOInit.h"

void InitDutySW(void);
uint16_t GetDuty(uint16_t RHU);


#endif /* PROG_DUTY_SW_H_ */
