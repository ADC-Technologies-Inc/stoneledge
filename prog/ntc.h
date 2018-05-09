/*
 * ptc.h
 *
 *  Created on: Feb 18, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_NTC_H_
#define PROG_NTC_H_

#include <stdint.h>

#include "../HW/Analog.h"

void SetNtcReady(void);
void ClearNtcReady(void);
uint16_t GetNtcReady(void);
void ProcessNtcSet(uint16_t set_);
uint16_t GetTempCycle(void);
uint32_t* GetTempData(void);
void InitNtc(void);


#endif /* PROG_NTC_H_ */
