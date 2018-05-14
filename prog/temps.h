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


uint16_t GetMaxTempData(void);
uint16_t GetTempDataSingle(uint16_t rhu_);
int ProcessTempData(void);
void EvaluateTempData(void);
void InitTemp(void);

#endif /* PROG_TEMPS_H_ */
