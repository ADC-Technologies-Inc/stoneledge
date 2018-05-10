/*
 * adct.h
 *
 *  Created on: Mar 7, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_ADCT_H_
#define PROG_ADCT_H_

#include <stdint.h>

#include "prog_conf.h"
#include "rhu.h"
#include "rhu_watchdog.h"
#include "LED.h"
#include "lcd.h"
#include "mux.h"
#include "temps.h"
#include "current.h"
#include "sys_id.h"
#include "../HW/Analog.h"
#include "../HW/IOInit.h"
#include "ntc.h"

void InitializeProgram(void);
void StartupTest(void);
void ControlLoop(void);
void SetMode(void);
Uint16 GetMode(void);

#endif /* PROG_ADCT_H_ */
