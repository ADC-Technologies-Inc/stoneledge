/*
 * ptc.c
 *
 *  Created on: Feb 18, 2018
 *      Author: Zack Lyzen
 */


#include "ntc.h"
#include "ntd_debug.h"

uint16_t *tempArraya0; 			// pointer to the temperatures just read by the ADC (Analog Channels Struct)
uint16_t *tempArraya1; 			// pointer to the temperatures a1
uint16_t *tempArraya2; 			// a2
uint16_t *tempArraya3; 			// a3
uint16_t *tempArraya4; 			// a4
uint16_t *tempArraya5; 			// a5
uint16_t *tempArraya6; 			// a6
uint16_t *tempArraya7; 			// a7
uint16_t *tempArrayb0; 			// b0
uint16_t *tempArrayb1; 			// b1
uint16_t *tempArrayb2; 			// b2

uint16_t *tempArrays[11];

uint32_t avgTempArray[176]; 	// temp readings are averaged and placed in this array for use
uint16_t tempCycle = 0; 		// tracks which input of the ADC MUX is currently being read
uint16_t tempCycleComplete = 0; // set to 1 at the end of each complete temp read, suggesting all array values have been populated and data is now valid
uint16_t ntcReady = 0;

void SetNtcReady(void)
{
	ntcReady = 1;
}

void ClearNtcReady(void)
{
	ntcReady = 0;
}

uint16_t GetNtcReady(void)
{
	return ntcReady;
}

/*0 based call (0...15 valid)*/
void ProcessNtcSet(uint16_t set_)
{
	uint16_t i = 0;
	uint32_t avg;

	while(i < 11)
	{
		avg = (tempArrays[i])[0];
		avg += (tempArrays[i])[1];
		avg += (tempArrays[i])[2];
		avg += (tempArrays[i])[3];
		avg += (tempArrays[i])[4];
		avg += (tempArrays[i])[5];
		avg += (tempArrays[i])[6];
		avg += (tempArrays[i])[7];
		avg += (tempArrays[i])[8];
		avg += (tempArrays[i])[9];
		avg /= 10;

		avgTempArray[((set_)*11)+i] = avg;
		i++;
	}
	tempCycle = set_;
}

uint16_t GetTempCycle(void)
{
	return tempCycle;
}

uint32_t* GetTempData(void)
{
	return avgTempArray;
}

void InitNtc(void)
{
	tempArraya0 = GetAnalogAddress(0);
	tempArraya1 = GetAnalogAddress(1);
	tempArraya2 = GetAnalogAddress(2);
	tempArraya3 = GetAnalogAddress(3);
	tempArraya4 = GetAnalogAddress(4);
	tempArraya5 = GetAnalogAddress(5);
	tempArraya6 = GetAnalogAddress(6);
	tempArraya7 = GetAnalogAddress(7);
	tempArrayb0 = GetAnalogAddress(8);
	tempArrayb1 = GetAnalogAddress(9);
	tempArrayb2 = GetAnalogAddress(10);

	tempArrays[0] = tempArraya0;
	tempArrays[1] = tempArraya1;
	tempArrays[2] = tempArraya2;
	tempArrays[3] = tempArraya3;
	tempArrays[4] = tempArraya4;
	tempArrays[5] = tempArraya5;
	tempArrays[6] = tempArraya6;
	tempArrays[7] = tempArraya7;
	tempArrays[8] = tempArrayb0;
	tempArrays[9] = tempArrayb1;
	tempArrays[10] = tempArrayb2;

}

