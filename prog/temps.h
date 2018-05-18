/*
 * temps.h
 *
 *  Created on: Mar 9, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_TEMPS_H_
#define PROG_TEMPS_H_

#include <stdint.h>
#include <math.h>

#include "../bits.h"
#include "ntc.h"
#include "prog_conf.h"


extern uint16_t cpu1_temp;
extern uint16_t cpu2_temp;

uint16_t GetMaxTempData(void);
uint16_t GetTempDataSingle(uint16_t rhu_);
uint16_t GetCPUTemp(uint16_t cpu_);

int ProcessTempData(void);
void EvaluateTempData(void);
void InitTemp(void);

#endif /* PROG_TEMPS_H_ */
