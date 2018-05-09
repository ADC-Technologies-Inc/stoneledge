/*
 * rhu_watchdog.h
 *
 *  Created on: Mar 8, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_RHU_WATCHDOG_H_
#define PROG_RHU_WATCHDOG_H_

#include <stdint.h>
#include "../bits.h"
#include "time.h"

#include "rhu.h"
#include "LED.h"
#include "lcd.h"


void FlagTcoError(Uint16 tco_);
Uint16 CritialError(void);
Uint16 CheckFuse(void);
void UpdateRhuWatchdog(void);
void ServiceRhuWatchdog(void);
void SetRhuWatchdog(Uint16 rhu_);
Uint16 GetRhuWatchdog(Uint16 rhu_);
void InitRhuWatchdog(void);

#endif /* PROG_RHU_WATCHDOG_H_ */
