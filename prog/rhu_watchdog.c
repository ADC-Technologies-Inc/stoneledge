/*
 * rhu_watchdog.c
 *
 *  Created on: Mar 8, 2018
 *      Author: Zack Lyzen
 */

#include "rhu_watchdog.h"

#define WD_DELAY 		5000 	// time off in ms for watchdog

struct RhuWd{
	Uint16 	rhu_num;
	Uint16 	prev_wd_time;
	Uint16 	current_wd_time;
	Uint16 	rhu_ongoing;
};

static struct  RhuWd Rhu1;
static struct  RhuWd Rhu2;
static struct  RhuWd Rhu3;
static struct  RhuWd Rhu4;
static struct  RhuWd Rhu5;
static struct  RhuWd Rhu6;
static struct  RhuWd Rhu7;
static struct  RhuWd Rhu8;

static struct RhuWd *Wds[8] = {&Rhu1, &Rhu2, &Rhu3, &Rhu4, &Rhu5, &Rhu6, &Rhu7, &Rhu8};

static Uint16 tcoErrorFlag[2] = {0,0};
static Uint16 rhuWatchdogs[9] = {0,0,0,0,0,0,0,0,0};


void FlagTcoError(Uint16 tco_)
{
	tcoErrorFlag[0] = tco_;
	tcoErrorFlag[1] = 1;
}

Uint16 CritialError(void)
{
	if(tcoErrorFlag[1] > 0)
		return 1;
	else
		return 0;
}

Uint16 CheckFuse(void)
{
	if(GpioDataRegs.GPADAT.bit.GPIO25 == 1)
		return 1;
	else
		return 0;
}

void UpdateRhuWatchdog(void)
{
	static int i;
	i = 0;
	while(i < 8)
	{
		if(rhuWatchdogs[i] && !(Wds[i]->rhu_ongoing))
		{
			Wds[i]->prev_wd_time = Wds[i]->current_wd_time;
			Wds[i]->current_wd_time = TIME_MS;
			Wds[i]->rhu_ongoing = 1;
			LedWdSet(i);
			LcdPostModal(8); 		// post overheat LCD message
		}
		i++;
	}
}

void ServiceRhuWatchdog(void)
{
	static int i;
	i = 0;
	while(i < 8)
	{
		if((Wds[i]->current_wd_time + WD_DELAY) < TIME_MS)
		{
			Wds[i]->rhu_ongoing = 0;
			rhuWatchdogs[i] = 0;
			LedWdClear(i);
		}
		i++;
	}
	rhuWatchdogs[8] = rhuWatchdogs[0] + rhuWatchdogs[1] + rhuWatchdogs[2] + rhuWatchdogs[3] + rhuWatchdogs[4] + rhuWatchdogs[5] + rhuWatchdogs[6] + rhuWatchdogs[7];
	if(rhuWatchdogs[8] > 0)
	{
		LedWdSet(LED_ONGOING);
	}
	else
	{
		LedWdClear(LED_ONGOING); 				// remove ongoing led
		LcdPostModal(9); 			// post recovered LCD message
	}
}

void SetRhuWatchdog(Uint16 rhu_)
{
	rhuWatchdogs[rhu_] = 1;
}

Uint16 GetRhuWatchdog(Uint16 rhu_)
{
	return rhuWatchdogs[rhu_];
}

void InitRhuWatchdog(void)
{
	static int i;
	i = 0;

	while(i < 8)
	{
		Wds[i]->current_wd_time = 0;
		Wds[i]->prev_wd_time = 0;
		Wds[i]->rhu_ongoing = 0;
		Wds[i]->rhu_num = i+1;
		i++;
	}
}
