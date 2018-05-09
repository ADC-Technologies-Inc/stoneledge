/*
 * sys_id.h
 *
 *  Created on: Mar 8, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_SYS_ID_H_
#define PROG_SYS_ID_H_

#include <stdint.h>

#include "prog_conf.h"
#include "../HW/IOInit.h"

void InitSysId(void);
Uint16 GetSysId(void);

#endif /* PROG_SYS_ID_H_ */
