/*
 * temps.c
 *
 *  Created on: Mar 9, 2018
 *      Author: Zack Lyzen
 */

#include "temps.h"

#define NUM_TEMPS 		10

#define C_TO_K 			(double)273
#define C_25_IN_K 		(double)298
#define RES_DIV 	 	(double)10000 		// resistor used for ntc divider (10k ohm) (also value of NTC @ 25C)

// temp variables

static double r_inf 	= 0; 		// r-inf calculation
static double exp_holder= 0;
static double B 		= 3900; 	// B value of NTC
static double R 		= 0; 		// measured resistance of NTC
static double tempDen 	= 0; 		// denominator of temp calc
static double tempNum 	= 0; 		// numerator of temp calc
static double T 		= 0; 		// result of temp calc in Kelvin

struct TempCell{
	uint16_t 	max_temp;
	uint16_t 	max_temp_counts;
	uint16_t 	watchdog_flagged;
	uint16_t 	wd_flag_time;
	uint16_t 	rhu_type; 			// right now 0 is permanent and 1 is removable, this will be changed to codes when Ethernet is brought online
	uint16_t 	ntc_error;
	uint16_t 	led_state;
};

static struct TempCell Cpu1;
static struct TempCell Cpu2;
static struct TempCell Misc;
static struct TempCell Ram;
static struct TempCell Dimm;
static struct TempCell Mtwo;
static struct TempCell Sff;
static struct TempCell Mezz;
static struct TempCell Board;
static struct TempCell Air;

static struct TempCell *Temps[NUM_TEMPS] = {&Cpu1, &Cpu2, &Misc, &Ram, &Dimm, &Mtwo, &Sff, &Mezz, &Board, &Air};

uint32_t *AverageTempArray;

uint16_t maxTempValue = 0;
uint16_t oldMaxTempValue = 0;
uint16_t maxTempValid = 0;

uint16_t GetMaxTempData(void)
{
	if(maxTempValid)
		return maxTempValue;
	else
		return oldMaxTempValue;
}

uint16_t GetTempDataSingle(uint16_t rhu_)
{
	return Temps[rhu_]->max_temp;
}

void SetMaxTemp(void)
{
	static int i;
	i = 0;
	oldMaxTempValue = maxTempValue;
	maxTempValid = 0;
	maxTempValue = 0;
	while(i <  NUM_TEMPS)
	{
		if(Temps[i]->max_temp > maxTempValue)
			maxTempValue = Temps[0]->max_temp;
		i++;
	}
	maxTempValid = 1;
}

void ProcessTempData(void)
{
	if(GetNtcReady() == 1)
	{
		ClearNtcReady();
		AverageTempArray = GetTempData();
		// CPU 1
		Temps[0]->max_temp_counts = AverageTempArray[44];
		if(Temps[0]->max_temp_counts < AverageTempArray[55])
			Temps[0]->max_temp_counts = AverageTempArray[55];
		if(Temps[0]->max_temp_counts < AverageTempArray[66])
			Temps[0]->max_temp_counts = AverageTempArray[66];
		if(Temps[0]->max_temp_counts < AverageTempArray[77])
			Temps[0]->max_temp_counts = AverageTempArray[77];
		if(Temps[0]->max_temp_counts < AverageTempArray[88])
			Temps[0]->max_temp_counts = AverageTempArray[88];
		// CPU 2
		Temps[1]->max_temp_counts = AverageTempArray[99];
		if(Temps[1]->max_temp_counts < AverageTempArray[110])
			Temps[1]->max_temp_counts = AverageTempArray[110];
		if(Temps[1]->max_temp_counts < AverageTempArray[121])
			Temps[1]->max_temp_counts = AverageTempArray[121];
		if(Temps[1]->max_temp_counts < AverageTempArray[132])
			Temps[1]->max_temp_counts = AverageTempArray[132];
		if(Temps[1]->max_temp_counts < AverageTempArray[143])
			Temps[1]->max_temp_counts = AverageTempArray[143];
		// MISC
		Temps[2]->max_temp_counts = AverageTempArray[89];
		if(Temps[2]->max_temp_counts < AverageTempArray[100])
			Temps[2]->max_temp_counts = AverageTempArray[100];
		if(Temps[2]->max_temp_counts < AverageTempArray[165])
			Temps[2]->max_temp_counts = AverageTempArray[165];
		// RAM
		Temps[3]->max_temp_counts = AverageTempArray[1];
		if(Temps[3]->max_temp_counts < AverageTempArray[2])
			Temps[3]->max_temp_counts = AverageTempArray[2];
		if(Temps[3]->max_temp_counts < AverageTempArray[12])
			Temps[3]->max_temp_counts = AverageTempArray[12];
		if(Temps[3]->max_temp_counts < AverageTempArray[13])
			Temps[3]->max_temp_counts = AverageTempArray[13];
		if(Temps[3]->max_temp_counts < AverageTempArray[23])
			Temps[3]->max_temp_counts = AverageTempArray[23];
		if(Temps[3]->max_temp_counts < AverageTempArray[24])
			Temps[3]->max_temp_counts = AverageTempArray[24];
		if(Temps[3]->max_temp_counts < AverageTempArray[34])
			Temps[3]->max_temp_counts = AverageTempArray[34];
		if(Temps[3]->max_temp_counts < AverageTempArray[35])
			Temps[3]->max_temp_counts = AverageTempArray[35];
		if(Temps[3]->max_temp_counts < AverageTempArray[45])
			Temps[3]->max_temp_counts = AverageTempArray[45];
		if(Temps[3]->max_temp_counts < AverageTempArray[46])
			Temps[3]->max_temp_counts = AverageTempArray[46];
		if(Temps[3]->max_temp_counts < AverageTempArray[56])
			Temps[3]->max_temp_counts = AverageTempArray[56];
		if(Temps[3]->max_temp_counts < AverageTempArray[57])
			Temps[3]->max_temp_counts = AverageTempArray[57];
		if(Temps[3]->max_temp_counts < AverageTempArray[67])
			Temps[3]->max_temp_counts = AverageTempArray[67];
		if(Temps[3]->max_temp_counts < AverageTempArray[68])
			Temps[3]->max_temp_counts = AverageTempArray[68];
		if(Temps[3]->max_temp_counts < AverageTempArray[78])
			Temps[3]->max_temp_counts = AverageTempArray[78];
		if(Temps[3]->max_temp_counts < AverageTempArray[79])
			Temps[3]->max_temp_counts = AverageTempArray[79];
		if(Temps[3]->max_temp_counts < AverageTempArray[90])
			Temps[3]->max_temp_counts = AverageTempArray[90];
		if(Temps[3]->max_temp_counts < AverageTempArray[101])
			Temps[3]->max_temp_counts = AverageTempArray[101];
		if(Temps[3]->max_temp_counts < AverageTempArray[111])
			Temps[3]->max_temp_counts = AverageTempArray[111];
		if(Temps[3]->max_temp_counts < AverageTempArray[122])
			Temps[3]->max_temp_counts = AverageTempArray[122];
		if(Temps[3]->max_temp_counts < AverageTempArray[133])
			Temps[3]->max_temp_counts = AverageTempArray[133];
		if(Temps[3]->max_temp_counts < AverageTempArray[144])
			Temps[3]->max_temp_counts = AverageTempArray[144];
		if(Temps[3]->max_temp_counts < AverageTempArray[155])
			Temps[3]->max_temp_counts = AverageTempArray[155];
		if(Temps[3]->max_temp_counts < AverageTempArray[166])
			Temps[3]->max_temp_counts = AverageTempArray[166];
		// DIMM
		Temps[4]->max_temp_counts = AverageTempArray[3];
		if(Temps[4]->max_temp_counts < AverageTempArray[4])
			Temps[4]->max_temp_counts = AverageTempArray[4];
		if(Temps[4]->max_temp_counts < AverageTempArray[14])
			Temps[4]->max_temp_counts = AverageTempArray[14];
		if(Temps[4]->max_temp_counts < AverageTempArray[15])
			Temps[4]->max_temp_counts = AverageTempArray[15];
		if(Temps[4]->max_temp_counts < AverageTempArray[25])
			Temps[4]->max_temp_counts = AverageTempArray[25];
		if(Temps[4]->max_temp_counts < AverageTempArray[26])
			Temps[4]->max_temp_counts = AverageTempArray[26];
		if(Temps[4]->max_temp_counts < AverageTempArray[36])
			Temps[4]->max_temp_counts = AverageTempArray[36];
		if(Temps[4]->max_temp_counts < AverageTempArray[37])
			Temps[4]->max_temp_counts = AverageTempArray[37];
		if(Temps[4]->max_temp_counts < AverageTempArray[47])
			Temps[4]->max_temp_counts = AverageTempArray[47];
		if(Temps[4]->max_temp_counts < AverageTempArray[48])
			Temps[4]->max_temp_counts = AverageTempArray[48];
		if(Temps[4]->max_temp_counts < AverageTempArray[58])
			Temps[4]->max_temp_counts = AverageTempArray[58];
		if(Temps[4]->max_temp_counts < AverageTempArray[59])
			Temps[4]->max_temp_counts = AverageTempArray[59];
		if(Temps[4]->max_temp_counts < AverageTempArray[69])
			Temps[4]->max_temp_counts = AverageTempArray[69];
		if(Temps[4]->max_temp_counts < AverageTempArray[70])
			Temps[4]->max_temp_counts = AverageTempArray[70];
		if(Temps[4]->max_temp_counts < AverageTempArray[80])
			Temps[4]->max_temp_counts = AverageTempArray[80];
		if(Temps[4]->max_temp_counts < AverageTempArray[81])
			Temps[4]->max_temp_counts = AverageTempArray[81];
		if(Temps[4]->max_temp_counts < AverageTempArray[91])
			Temps[4]->max_temp_counts = AverageTempArray[91];
		if(Temps[4]->max_temp_counts < AverageTempArray[92])
			Temps[4]->max_temp_counts = AverageTempArray[92];
		if(Temps[4]->max_temp_counts < AverageTempArray[102])
			Temps[4]->max_temp_counts = AverageTempArray[102];
		if(Temps[4]->max_temp_counts < AverageTempArray[103])
			Temps[4]->max_temp_counts = AverageTempArray[103];
		if(Temps[4]->max_temp_counts < AverageTempArray[112])
			Temps[4]->max_temp_counts = AverageTempArray[112];
		if(Temps[4]->max_temp_counts < AverageTempArray[113])
			Temps[4]->max_temp_counts = AverageTempArray[113];
		if(Temps[4]->max_temp_counts < AverageTempArray[123])
			Temps[4]->max_temp_counts = AverageTempArray[123];
		if(Temps[4]->max_temp_counts < AverageTempArray[124])
			Temps[4]->max_temp_counts = AverageTempArray[124];
		if(Temps[4]->max_temp_counts < AverageTempArray[134])
			Temps[4]->max_temp_counts = AverageTempArray[134];
		if(Temps[4]->max_temp_counts < AverageTempArray[135])
			Temps[4]->max_temp_counts = AverageTempArray[135];
		if(Temps[4]->max_temp_counts < AverageTempArray[145])
			Temps[4]->max_temp_counts = AverageTempArray[145];
		if(Temps[4]->max_temp_counts < AverageTempArray[146])
			Temps[4]->max_temp_counts = AverageTempArray[146];
		if(Temps[4]->max_temp_counts < AverageTempArray[156])
			Temps[4]->max_temp_counts = AverageTempArray[156];
		if(Temps[4]->max_temp_counts < AverageTempArray[157])
			Temps[4]->max_temp_counts = AverageTempArray[157];
		if(Temps[4]->max_temp_counts < AverageTempArray[167])
			Temps[4]->max_temp_counts = AverageTempArray[167];
		if(Temps[4]->max_temp_counts < AverageTempArray[168])
			Temps[4]->max_temp_counts = AverageTempArray[168];
		// M.2
		Temps[5]->max_temp_counts = AverageTempArray[5];
		if(Temps[5]->max_temp_counts < AverageTempArray[16])
			Temps[5]->max_temp_counts = AverageTempArray[16];
		if(Temps[5]->max_temp_counts < AverageTempArray[27])
			Temps[5]->max_temp_counts = AverageTempArray[27];
		if(Temps[5]->max_temp_counts < AverageTempArray[38])
			Temps[5]->max_temp_counts = AverageTempArray[38];
		if(Temps[5]->max_temp_counts < AverageTempArray[49])
			Temps[5]->max_temp_counts = AverageTempArray[49];
		if(Temps[5]->max_temp_counts < AverageTempArray[60])
			Temps[5]->max_temp_counts = AverageTempArray[60];
		if(Temps[5]->max_temp_counts < AverageTempArray[71])
			Temps[5]->max_temp_counts = AverageTempArray[71];
		if(Temps[5]->max_temp_counts < AverageTempArray[82])
			Temps[5]->max_temp_counts = AverageTempArray[82];
		if(Temps[5]->max_temp_counts < AverageTempArray[93])
			Temps[5]->max_temp_counts = AverageTempArray[93];
		if(Temps[5]->max_temp_counts < AverageTempArray[104])
			Temps[5]->max_temp_counts = AverageTempArray[104];
		if(Temps[5]->max_temp_counts < AverageTempArray[115])
			Temps[5]->max_temp_counts = AverageTempArray[115];
		if(Temps[5]->max_temp_counts < AverageTempArray[126])
			Temps[5]->max_temp_counts = AverageTempArray[126];
		if(Temps[5]->max_temp_counts < AverageTempArray[137])
			Temps[5]->max_temp_counts = AverageTempArray[137];
		if(Temps[5]->max_temp_counts < AverageTempArray[148])
			Temps[5]->max_temp_counts = AverageTempArray[148];
		if(Temps[5]->max_temp_counts < AverageTempArray[159])
			Temps[5]->max_temp_counts = AverageTempArray[159];
		if(Temps[5]->max_temp_counts < AverageTempArray[170])
			Temps[5]->max_temp_counts = AverageTempArray[170];
		// SFF
		Temps[6]->max_temp_counts = AverageTempArray[114];
		if(Temps[6]->max_temp_counts < AverageTempArray[125])
			Temps[6]->max_temp_counts = AverageTempArray[125];
		if(Temps[6]->max_temp_counts < AverageTempArray[136])
			Temps[6]->max_temp_counts = AverageTempArray[136];
		if(Temps[6]->max_temp_counts < AverageTempArray[147])
			Temps[6]->max_temp_counts = AverageTempArray[147];
		// MEZZ
		Temps[7]->max_temp_counts = AverageTempArray[98];
		if(Temps[7]->max_temp_counts < AverageTempArray[109])
			Temps[7]->max_temp_counts = AverageTempArray[109];
		// Board
		Temps[8]->max_temp_counts = AverageTempArray[6];
		if(Temps[8]->max_temp_counts < AverageTempArray[7])
			Temps[8]->max_temp_counts = AverageTempArray[7];
		if(Temps[8]->max_temp_counts < AverageTempArray[17])
			Temps[8]->max_temp_counts = AverageTempArray[17];
		if(Temps[8]->max_temp_counts < AverageTempArray[18])
			Temps[8]->max_temp_counts = AverageTempArray[18];
		if(Temps[8]->max_temp_counts < AverageTempArray[28])
			Temps[8]->max_temp_counts = AverageTempArray[28];
		if(Temps[8]->max_temp_counts < AverageTempArray[29])
			Temps[8]->max_temp_counts = AverageTempArray[29];
		if(Temps[8]->max_temp_counts < AverageTempArray[39])
			Temps[8]->max_temp_counts = AverageTempArray[39];
		if(Temps[8]->max_temp_counts < AverageTempArray[40])
			Temps[8]->max_temp_counts = AverageTempArray[40];
		if(Temps[8]->max_temp_counts < AverageTempArray[50])
			Temps[8]->max_temp_counts = AverageTempArray[50];
		if(Temps[8]->max_temp_counts < AverageTempArray[51])
			Temps[8]->max_temp_counts = AverageTempArray[51];
		if(Temps[8]->max_temp_counts < AverageTempArray[61])
			Temps[8]->max_temp_counts = AverageTempArray[61];
		if(Temps[8]->max_temp_counts < AverageTempArray[62])
			Temps[8]->max_temp_counts = AverageTempArray[62];
		if(Temps[8]->max_temp_counts < AverageTempArray[72])
			Temps[8]->max_temp_counts = AverageTempArray[72];
		if(Temps[8]->max_temp_counts < AverageTempArray[73])
			Temps[8]->max_temp_counts = AverageTempArray[73];
		if(Temps[8]->max_temp_counts < AverageTempArray[83])
			Temps[8]->max_temp_counts = AverageTempArray[83];
		if(Temps[8]->max_temp_counts < AverageTempArray[84])
			Temps[8]->max_temp_counts = AverageTempArray[84];
		if(Temps[8]->max_temp_counts < AverageTempArray[94])
			Temps[8]->max_temp_counts = AverageTempArray[94];
		if(Temps[8]->max_temp_counts < AverageTempArray[95])
			Temps[8]->max_temp_counts = AverageTempArray[95];
		if(Temps[8]->max_temp_counts < AverageTempArray[97])
			Temps[8]->max_temp_counts = AverageTempArray[97];
		if(Temps[8]->max_temp_counts < AverageTempArray[105])
			Temps[8]->max_temp_counts = AverageTempArray[105];
		if(Temps[8]->max_temp_counts < AverageTempArray[106])
			Temps[8]->max_temp_counts = AverageTempArray[106];
		if(Temps[8]->max_temp_counts < AverageTempArray[108])
			Temps[8]->max_temp_counts = AverageTempArray[108];
		if(Temps[8]->max_temp_counts < AverageTempArray[116])
			Temps[8]->max_temp_counts = AverageTempArray[116];
		if(Temps[8]->max_temp_counts < AverageTempArray[117])
			Temps[8]->max_temp_counts = AverageTempArray[117];
		if(Temps[8]->max_temp_counts < AverageTempArray[119])
			Temps[8]->max_temp_counts = AverageTempArray[119];
		if(Temps[8]->max_temp_counts < AverageTempArray[127])
			Temps[8]->max_temp_counts = AverageTempArray[127];
		if(Temps[8]->max_temp_counts < AverageTempArray[128])
			Temps[8]->max_temp_counts = AverageTempArray[128];
		if(Temps[8]->max_temp_counts < AverageTempArray[130])
			Temps[8]->max_temp_counts = AverageTempArray[130];
		if(Temps[8]->max_temp_counts < AverageTempArray[138])
			Temps[8]->max_temp_counts = AverageTempArray[138];
		if(Temps[8]->max_temp_counts < AverageTempArray[139])
			Temps[8]->max_temp_counts = AverageTempArray[139];
		if(Temps[8]->max_temp_counts < AverageTempArray[141])
			Temps[8]->max_temp_counts = AverageTempArray[141];
		if(Temps[8]->max_temp_counts < AverageTempArray[149])
			Temps[8]->max_temp_counts = AverageTempArray[149];
		if(Temps[8]->max_temp_counts < AverageTempArray[150])
			Temps[8]->max_temp_counts = AverageTempArray[150];
		if(Temps[8]->max_temp_counts < AverageTempArray[152])
			Temps[8]->max_temp_counts = AverageTempArray[152];
		if(Temps[8]->max_temp_counts < AverageTempArray[160])
			Temps[8]->max_temp_counts = AverageTempArray[160];
		if(Temps[8]->max_temp_counts < AverageTempArray[161])
			Temps[8]->max_temp_counts = AverageTempArray[161];
		if(Temps[8]->max_temp_counts < AverageTempArray[163])
			Temps[8]->max_temp_counts = AverageTempArray[163];
		if(Temps[8]->max_temp_counts < AverageTempArray[171])
			Temps[8]->max_temp_counts = AverageTempArray[171];
		if(Temps[8]->max_temp_counts < AverageTempArray[172])
			Temps[8]->max_temp_counts = AverageTempArray[172];
		if(Temps[8]->max_temp_counts < AverageTempArray[174])
			Temps[8]->max_temp_counts = AverageTempArray[174];
		// Air
		Temps[9]->max_temp_counts = AverageTempArray[9];
		if(Temps[9]->max_temp_counts < AverageTempArray[10])
			Temps[9]->max_temp_counts = AverageTempArray[10];
		if(Temps[9]->max_temp_counts < AverageTempArray[20])
			Temps[9]->max_temp_counts = AverageTempArray[20];
		if(Temps[9]->max_temp_counts < AverageTempArray[21])
			Temps[9]->max_temp_counts = AverageTempArray[21];
		if(Temps[9]->max_temp_counts < AverageTempArray[31])
			Temps[9]->max_temp_counts = AverageTempArray[31];
		if(Temps[9]->max_temp_counts < AverageTempArray[32])
			Temps[9]->max_temp_counts = AverageTempArray[32];
		if(Temps[9]->max_temp_counts < AverageTempArray[42])
			Temps[9]->max_temp_counts = AverageTempArray[42];
		if(Temps[9]->max_temp_counts < AverageTempArray[43])
			Temps[9]->max_temp_counts = AverageTempArray[43];
		if(Temps[9]->max_temp_counts < AverageTempArray[53])
			Temps[9]->max_temp_counts = AverageTempArray[53];
		if(Temps[9]->max_temp_counts < AverageTempArray[54])
			Temps[9]->max_temp_counts = AverageTempArray[54];
		if(Temps[9]->max_temp_counts < AverageTempArray[64])
			Temps[9]->max_temp_counts = AverageTempArray[64];
		if(Temps[9]->max_temp_counts < AverageTempArray[65])
			Temps[9]->max_temp_counts = AverageTempArray[65];
		if(Temps[9]->max_temp_counts < AverageTempArray[75])
			Temps[9]->max_temp_counts = AverageTempArray[75];
		if(Temps[9]->max_temp_counts < AverageTempArray[76])
			Temps[9]->max_temp_counts = AverageTempArray[76];
		if(Temps[9]->max_temp_counts < AverageTempArray[86])
			Temps[9]->max_temp_counts = AverageTempArray[86];
		if(Temps[9]->max_temp_counts < AverageTempArray[87])
			Temps[9]->max_temp_counts = AverageTempArray[87];
		if(Temps[9]->max_temp_counts < AverageTempArray[96])
			Temps[9]->max_temp_counts = AverageTempArray[96];
		if(Temps[9]->max_temp_counts < AverageTempArray[107])
			Temps[9]->max_temp_counts = AverageTempArray[107];
		if(Temps[9]->max_temp_counts < AverageTempArray[118])
			Temps[9]->max_temp_counts = AverageTempArray[118];
		if(Temps[9]->max_temp_counts < AverageTempArray[129])
			Temps[9]->max_temp_counts = AverageTempArray[129];
		if(Temps[9]->max_temp_counts < AverageTempArray[140])
			Temps[9]->max_temp_counts = AverageTempArray[140];
		if(Temps[9]->max_temp_counts < AverageTempArray[151])
			Temps[9]->max_temp_counts = AverageTempArray[151];
		if(Temps[9]->max_temp_counts < AverageTempArray[162])
			Temps[9]->max_temp_counts = AverageTempArray[162];
		if(Temps[9]->max_temp_counts < AverageTempArray[173])
			Temps[9]->max_temp_counts = AverageTempArray[173];

		SetMaxTemp();
	}
}

uint16_t ConvertTemp(uint16_t counts_)
{
	tempNum = RES_DIV*(double)(4096 - counts_);
	tempNum /= (double)4096;
	tempDen = (double)(4096 - counts_)/(double)4096;
	tempDen = (double)1 - tempDen;
	R = tempNum / tempDen; 					// measured resistance value of NTC
	tempNum = B; 							// numerator is only Beta
	exp_holder = (-1 * B) / C_25_IN_K; 		// exponent of exponential portion of r_inf
	exp_holder = exp(exp_holder); 			// raises e to 'exp_holder'
	r_inf = RES_DIV * exp_holder; 			// r_inf used to calculate temp
	tempDen = log(R / r_inf); 				// used natural log to calculate the denominator of temp calc
	T = tempNum / tempDen; 					// temp in K
	T -= C_TO_K; 							// converts to C
	T *= 10; 								// converts to C * 10
	return (uint16_t)T; 					// returns at uint16_t
}

void EvaluateTempData(void)
{
	static int i;
	i = 0;
	while(i < NUM_TEMPS - 2)
	{
		Temps[i]->max_temp = ConvertTemp(Temps[i]->max_temp_counts); 		// convert measurement from counts to temp

		if(Temps[i]->max_temp > RHU_TEMP_MAX_LIMIT) 				//
		{
			SetRhuWatchdog(i);
		}
		i++;
	}
	if(Temps[8]->max_temp > BOARD_TEMP_MAX_LIMIT)
	{
		//
	}
	if(Temps[9]->max_temp > AIR_TEMP_MAX_LIMIT)
	{
		//
	}

}

void InitTemp(void)
{
	static int i;
	i = 0;

	while(i<NUM_TEMPS)
	{
		Temps[i]->led_state = 0;
		Temps[i]->ntc_error = 0;
		Temps[i]->watchdog_flagged = 0;
		Temps[i]->wd_flag_time = 0;
		Temps[i]->rhu_type = 0;
		i++;
	}
	Temps[9]->rhu_type = 1;
}

