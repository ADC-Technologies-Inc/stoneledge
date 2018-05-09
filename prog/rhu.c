/*
 * rhu.c
 *
 *  Created on: Feb 25, 2018
 *      Author: Zack Lyzen
 */

//===========================================================================
// Includes
//===========================================================================

#include "../prog/rhu.h"
//===========================================================================
// private variables
//===========================================================================

#ifdef LOW_DUTY_MODE
static Uint16 duties[4] = {0, 50, 75, 100};
#else
static Uint16 duties[4] = {0, 500, 75, 1000};
#endif
static Uint16 tcos[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static Uint16 tco_gpio[8] = {504, 505, 506, 507, 600, 601, 602, 603};
static Uint16 rhus[8] = {CPU1, CPU2, MISC, RAM, DIMM_GRP, M_2_GRP, SFF_GRP, MEZZ};
static Uint16 duty_cycle_settings[8] = {0,0,0,0,0,0,0,0}; 		// array of duty cycle settings for each RHU
static Uint16 rhu_en_array  = 0; 								// bit array, 0 is disabled RHU and 1 is enabled RHU


/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Running
 *
 -----------------------------------------------------------------------------------------------------------*/

void EnableRhuRelay(void)
{
	GpioDataRegs.GPBSET.bit.GPIO40 = 0x01;
}

void DisableRhuRelay(void)
{
	GpioDataRegs.GPBCLEAR.bit.GPIO40 = 0x01;
}

Uint16 GetRhuRelayState(void)
{
	return GpioDataRegs.GPBDAT.bit.GPIO40;
}

void CheckTco(Uint16 tco_, Uint16 duty_cycle_)
{
	if(duty_cycle_>0)
	{
		if(ExtGpioRead(tco_gpio[tco_]) == 0)
		{
			tcos[tco_] = 1;
		}
	}
	else
	{
		tcos[tco_] = 0;
	}
}

void CheckTcoResults(void)
{
	static int i;
	i = 0;
	while(i < 8)
	{
		if(tcos[i] == 1)
		{
			FlagTcoError(i);
		}
		i++;
	}
}

void ProcessRhuRunning(void)
{
	static int i;
	i = 0;
	while(i < RHU_QTY)
	{
		if((rhu_en_array & bit_arrays[i]) && !(GetRhuWatchdog(i))) 	// if RHU(i+1) is enabled
		{
			SetDuty((i+1), (duty_cycle_settings[i])); 				// .. turn on with requested duty cycle
		}
		else 														// if RHU(i+1) is disabled
		{
			SetDuty((i+1), 0); 										// .. set duty cycle to 0% (turn off)
		}
		i++;
	}
}

void EnableRhu(uint16_t rhu_)
{
	rhu_en_array |= (bit_arrays[rhu_-1]);
}

void DisableRhu(uint16_t rhu_)
{
	rhu_en_array &= ~(bit_arrays[rhu_-1]);
}

void EstopRhu(void)
{
	static int i;
	i = 0;

	rhu_en_array = 0;
	while(i < RHU_QTY)
	{
		SetDuty((i+1), 0);
		i++;
	}
	// all duty cycles 0
	DisableRhuRelay();
}

/*-----------------------------------------------------------------------------------------------------------
 *
 * Functions - Init
 *
 -----------------------------------------------------------------------------------------------------------*/

void SetRhuDutyArray(uint16_t rhu_, uint16_t duty_)
{
	duty_cycle_settings[rhu_-1] = duty_;
}

void InitializeRHUs(void)
{
	static uint16_t temp_duty;
	temp_duty = 0;

	// RHU 1
	if(ExtGpioRead(300) == 1)
	{
		temp_duty = 0x02;
	}
	if(ExtGpioRead(301) == 1)
	{
		temp_duty += 0x01;
	}
	if(!USE_RHU_1)
		temp_duty = 0;
	SetRhuDutyArray(rhus[0], duties[temp_duty]);


	// RHU 2
	temp_duty = 0;
	if(ExtGpioRead(302) == 1)
	{
		temp_duty = 0x02;
	}
	if(ExtGpioRead(303) == 1)
	{
		temp_duty += 0x01;
	}
	if(!USE_RHU_2)
		temp_duty = 0;
	SetRhuDutyArray(rhus[1], duties[temp_duty]);

	// RHU 3
	temp_duty = 0;
	if(ExtGpioRead(304) == 1)
	{
		temp_duty = 0x02;
	}
	if(ExtGpioRead(305) == 1)
	{
		temp_duty += 0x01;
	}
	if(!USE_RHU_3)
		temp_duty = 0;
	SetRhuDutyArray(rhus[2], duties[temp_duty]);

	// RHU 4
	temp_duty = 0;
	if(ExtGpioRead(306) == 1)
	{
		temp_duty = 0x02;
	}
	if(ExtGpioRead(307) == 1)
	{
		temp_duty += 0x01;
	}
	if(!USE_RHU_4)
		temp_duty = 0;
	SetRhuDutyArray(rhus[3], duties[temp_duty]);

	// RHU 5
	temp_duty = 0;
	if(ExtGpioRead(400) == 1)
	{
		temp_duty = 0x02;
	}
	if(ExtGpioRead(401) == 1)
	{
		temp_duty += 0x01;
	}
	if(!USE_RHU_5)
		temp_duty = 0;
	SetRhuDutyArray(rhus[4], duties[temp_duty]);

	// RHU 6
	temp_duty = 0;
	if(ExtGpioRead(402) == 1)
	{
		temp_duty = 0x02;
	}
	if(ExtGpioRead(403) == 1)
	{
		temp_duty += 0x01;
	}
	if(!USE_RHU_6)
		temp_duty = 0;
	SetRhuDutyArray(rhus[5], duties[temp_duty]);

	temp_duty = 0;
	if(ExtGpioRead(404) == 1)
	{
		temp_duty = 0x02;
	}
	if(ExtGpioRead(405) == 1)
	{
		temp_duty += 0x01;
	}
	if(!USE_RHU_7)
		temp_duty = 0;
	SetRhuDutyArray(rhus[6], duties[temp_duty]);

	// RHU 8
	temp_duty = 0;
	if(ExtGpioRead(406) == 1)
	{
		temp_duty = 0x02;
	}
	if(ExtGpioRead(407) == 1)
	{
		temp_duty += 0x01;
	}
	if(!USE_RHU_8)
		temp_duty = 0;
	SetRhuDutyArray(rhus[7], duties[temp_duty]);
}


