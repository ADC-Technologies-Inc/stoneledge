/*
 * ptc.c
 *
 *  Created on: Feb 18, 2018
 *      Author: Zack Lyzen
 */


#include "ntc.h"

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

void ProcessNtcSet(uint16_t set_)
{
	static uint16_t i;
	i = 0;

	while(i < 11)
	{
		avgTempArray[((set_-1)*11)+i] = (tempArrays[i])[0];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[1];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[2];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[3];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[4];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[5];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[6];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[7];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[8];
		avgTempArray[((set_-1)*11)+i] += (tempArrays[i])[9];
		avgTempArray[((set_-1)*11)+i] /= 10;
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

