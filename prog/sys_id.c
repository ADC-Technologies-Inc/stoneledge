/*
 * sys_id.c
 *
 *  Created on: Mar 8, 2018
 *      Author: Zack Lyzen
 */

//#define DEBUG_SYS_ID

#include <stdint.h>
#include "../prog/ntd_debug.h"

#include "sys_id.h"

Uint16 sysId = 0;

void InitSysId(void)
{
	if(ExtGpioRead(800) == 1)
	{
		sysId += 0x0001;
	}
	if(ExtGpioRead(801) == 1)
	{
		sysId += 0x0002;
	}
	if(ExtGpioRead(802) == 1)
	{
		sysId += 0x0004;
	}
	if(ExtGpioRead(803) == 1)
	{
		sysId += 0x0008;
	}
	if(ExtGpioRead(804) == 1)
	{
		sysId += 0x0010;
	}
	if(ExtGpioRead(805) == 1)
	{
		sysId += 0x0020;
	}
	if(ExtGpioRead(806) == 1)
	{
		sysId += 0x0040;
	}
	if(ExtGpioRead(807) == 1)
	{
		sysId += 0x0080;
	}
	if(ExtGpioRead(704) == 1)
	{
		sysId += 0x0100;
	}
	if(ExtGpioRead(705) == 1)
	{
		sysId += 0x0200;
	}

#ifdef DEBUG_SYS_ID
	printf("InitSysId: "PRINTF_BINSTR8"\n", PRINTF_BINSTR8_ARGS(sysId) );
#endif
}

Uint16 GetSysId(void)
{
	return sysId;
}
