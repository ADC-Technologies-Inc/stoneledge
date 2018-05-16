/*
 * current.c
 *
 *  Created on: Mar 7, 2018
 *      Author: Zack Lyzen
 */

#include "current.h"

uint16_t *currentArray; 			// pointer to the current just read by the ADC (Analog Channels Struct)

uint32_t avgCurrentArray[16]; 		// holds current averages (over 16 MUX channel switches)

uint32_t avgCurrentCalc;
uint32_t avgPowerCalc;


void ProcessCurrentSet(uint16_t set_)
{
	avgCurrentArray[set_]  = currentArray[0];
	avgCurrentArray[set_] += currentArray[1];
	avgCurrentArray[set_] += currentArray[2];
	avgCurrentArray[set_] += currentArray[3];
	avgCurrentArray[set_] += currentArray[4];
	avgCurrentArray[set_] += currentArray[5];
	avgCurrentArray[set_] += currentArray[6];
	avgCurrentArray[set_] += currentArray[7];
	avgCurrentArray[set_] += currentArray[8];
	avgCurrentArray[set_] += currentArray[9];
	avgCurrentArray[set_] /= 10;
}

uint32_t GetCurrentData(void)
{
    double Amps;

	avgCurrentCalc  = avgCurrentArray[0];
	avgCurrentCalc += avgCurrentArray[1];
	avgCurrentCalc += avgCurrentArray[2];
	avgCurrentCalc += avgCurrentArray[3];
	avgCurrentCalc += avgCurrentArray[4];
	avgCurrentCalc += avgCurrentArray[5];
	avgCurrentCalc += avgCurrentArray[6];
	avgCurrentCalc += avgCurrentArray[7];
	avgCurrentCalc += avgCurrentArray[8];
	avgCurrentCalc += avgCurrentArray[9];
	avgCurrentCalc += avgCurrentArray[10];
	avgCurrentCalc += avgCurrentArray[11];
	avgCurrentCalc += avgCurrentArray[12];
	avgCurrentCalc += avgCurrentArray[13];
	avgCurrentCalc += avgCurrentArray[14];
	avgCurrentCalc += avgCurrentArray[15];
	avgCurrentCalc /= 16;

	//3.5 figure is bothersome! - this is incredibly inaccurate because of it
	//Amps = (4096/0.175)*((double) avgCurrentCalc/3.5);
	//avgCurrentCalc = (uint32_t)(Amps * 10);


	avgCurrentCalc *= 10; 					// values used to convert ADC counts to amps * 10
	avgCurrentCalc /= 217; 					//

	return avgCurrentCalc;
}

uint32_t GetPowerData(void)
{
	avgPowerCalc = 48 * (GetCurrentData()); // 48V measurement not performed at this time, assume 48V power supply
	avgPowerCalc /= 10; 					// calculates power based on current (amps * 10)
	return avgPowerCalc;
}

void InitCurrent(void)
{
	currentArray = GetAnalogAddress(11);
}
