/*
 * adct.h
 *
 *  Created on: Mar 7, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_CTL_H_
#define PROG_CTL_H_

#include <stdint.h>

#include "prog_conf.h"
#include "rhu.h"
#include "LED.h"
#include "lcd.h"
#include "mux.h"
#include "temps.h"
#include "current.h"
#include "sys_id.h"
#include "../HW/Analog.h"
#include "../HW/IOInit.h"
#include "ntc.h"

extern int online;  //set when entering online mode, allows ePWM4 to operate

//Entry call
void CTL_Enter(void);

//Callback from ePWM4 for priority online control code
void CTL_OnlineCALLBACK(void);

//System Wide HardSTOP, call this to bring the system to a stop after doing any specific module related tasks
void CTL_HardSTOP( uint16_t msg_);

#endif /* PROG_CTL_H_ */
