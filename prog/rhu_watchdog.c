/*
 * rhu_watchdog.c
 *
 *  Created on: Mar 8, 2018
 *      Author: Zack Lyzen
 */

#include "rhu_watchdog.h"

#define DEBUG_RHU_WATCHDOG

#ifdef DEBUG_RHU_WATCHDOG
#include "ntd_debug.h"
#endif

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
#ifdef DEBUG_RHU_WATCHDOG
        printf("FlagTcoError():: Flagging for TCO %d\n", tco_);
#endif
	tcoErrorFlag[0] = tco_;
	tcoErrorFlag[1] = 1;
}

Uint16 CriticalError(void)
{
	if(tcoErrorFlag[1] > 0){
#ifdef DEBUG_RHU_WATCHDOG
	    printf("CriticalError():: Error Flagged\n");
#endif
		return 1;
	}else{
#ifdef DEBUG_RHU_WATCHDOG
        printf("CriticalError():: Error OK\n");
#endif
		return 0;
	}
}

/*Verify 48V has power (reads from 48V fuse line, before the relays)*/
Uint16 CheckFuse(void)
{
	if(GpioDataRegs.GPADAT.bit.GPIO25 == 1){
#ifdef DEBUG_RHU_WATCHDOG
        printf("CheckFuse():: 48v Power OK\n");
#endif
		return 1;
	}else{
#ifdef DEBUG_RHU_WATCHDOG
        printf("CheckFuse():: 48v Power Not Available\n");
#endif
		return 0;
	}
}

/*Walk through the watchdogs list and check if anything's changed (populated by??)*/
void UpdateRhuWatchdog(void)
{
#ifdef DEBUG_RHU_WATCHDOG
        printf("UpdateRhuWatchdog():: Checking for flagged errors\n");
#endif

	int i = 0;
	while(i < 8)
	{
		if(rhuWatchdogs[i] && !(Wds[i]->rhu_ongoing))
		{
			Wds[i]->prev_wd_time = Wds[i]->current_wd_time;
			Wds[i]->current_wd_time = get_time_ms();
			Wds[i]->rhu_ongoing = 1;

#ifdef DEBUG_RHU_WATCHDOG
        printf("UpdateRhuWatchdog():: New error flagged in RHU %d, setting LED and posting overheat message\n", i);
#endif

			LED_Set(i);
			LcdPostModal(8); 		// post overheat LCD message
		}
		i++;
	}
}

void ServiceRhuWatchdog(void)
{
	int i = 0;

#ifdef DEBUG_RHU_WATCHDOG
        printf("ServiceRhuWatchdog():: Servicing RhuWatchdogs\n");
#endif

	for(i=0;i < 8;i++){
		if((Wds[i]->current_wd_time + WD_DELAY) < get_time_ms() ){
#ifdef DEBUG_RHU_WATCHDOG
		    printf("ServiceRhuWatchdog():: Overheat no longer present for RHU %d, clearing flag and resetting LED\n", i);
#endif

			Wds[i]->rhu_ongoing = 0;
			rhuWatchdogs[i] = 0;
			LED_Clear(i);
		}
	}
	rhuWatchdogs[8] = rhuWatchdogs[0] + rhuWatchdogs[1] + rhuWatchdogs[2] + rhuWatchdogs[3] + rhuWatchdogs[4] + rhuWatchdogs[5] + rhuWatchdogs[6] + rhuWatchdogs[7];
	if(rhuWatchdogs[8] > 0)	{
#ifdef DEBUG_RHU_WATCHDOG
            printf("ServiceRhuWatchdog():: Ongoing Error condition, setting LED\n");
#endif

		LED_Set(LED_ONGOING);
	}else{
#ifdef DEBUG_RHU_WATCHDOG
            printf("ServiceRhuWatchdog():: Clearing ongoing error condition, clearing LED and posting recovered msg\n");
#endif

		LED_Clear(LED_ONGOING); 				// remove ongoing led
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
