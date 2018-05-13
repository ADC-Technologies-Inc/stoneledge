/*
 * temps.c
 *
 *  Created on: Mar 9, 2018
 *      Author: Zack Lyzen
 */

//#define DEBUG_TEMPS

#ifdef DEBUG_TEMPS
#include "ntd_debug.h"
#endif

#include "temps.h"

#define NUM_TEMPS 		10

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

static uint16_t max_temp = 0;


#define C_TO_K          (double)273
#define C_25_IN_K       (double)298
#define RES_DIV         (double)10000            // resistor used for ntc divider (10k ohm) (also value of NTC @ 25C)

//Beta and r_inf values from datasheets for each Thermistor type
#define BETA_JT         (double) 3435            //Semitec 103JT-xxx
#define RINF_JT         (double) 0.0991911898

#define BETA_SC30       (double) 3981            //Amphenol SC30F103V
#define RINF_SC30       (double) 0.0158910527

#define BETA_NCP        (double) 3380            //muRata NCP18XH103F03RB
#define RINF_NCP        (double) 0.1192855385


//Macro to simplify calling the converter
#define CONVERTTEMP_JT( counts_ ) ConvertTemp_Generic(counts_, BETA_JT, RINF_JT )
#define CONVERTTEMP_SC30( counts_ ) ConvertTemp_Generic(counts_, BETA_SC30, RINF_SC30 )
#define CONVERTTEMP_NCP( counts_ ) ConvertTemp_Generic(counts_, BETA_NCP, RINF_NCP )

uint16_t ConvertTemp_Generic(uint16_t counts_, double beta_, double r_inf_);


uint16_t GetMaxTempData(void){
   return max_temp;
}

/*rhu_ is 0-based*/
uint16_t GetTempDataSingle(uint16_t rhu_)
{
	return Temps[rhu_]->max_temp;
}


#ifdef DEBUG_TEMPS
#define TESTANDSET_MAXTEMP( arr_, idx_) if(Temps[arr_]->max_temp_counts < AverageTempArray[idx_] ) Temps[arr_]->max_temp_counts = AverageTempArray[idx_];\
    printf("IDX: %d COUNTS: %d, TEMP: %d\n",idx_, (uint16_t) AverageTempArray[idx_], CONVERTTEMP_NCP(AverageTempArray[idx_]));
#else
#define TESTANDSET_MAXTEMP( arr_, idx_) if(Temps[arr_]->max_temp_counts < AverageTempArray[idx_] ) Temps[arr_]->max_temp_counts = AverageTempArray[idx_];
#endif

/*
 * RETURNS
 *
 * -1 = NOT READY
 * 0 = TEMPS GOOD
 * >0 OTHER, bitfield of relevant RHUs that are out of spec as defined below
 *
 * CPU1                        0
 * CPU2                        1
 * MISC                        2
 * RAM                         3
 * DIMM_GRP                    4
 * M_2_GRP                     5
 * SFF_GRP                     6
 * MEZZ                        7
 * BOARD                       8
 * AIR                         9
 *
 * */
int ProcessTempData(void)
{
    /*NTD - VERIFIED 05/10/18. ALL IDX ACCURATE*/

    uint16_t sc30_temp, i;
    int err = 0;

	if(GetNtcReady() == 1)
	{
		ClearNtcReady();
		AverageTempArray = GetTempData();
		// CPU 1
		Temps[0]->max_temp_counts = AverageTempArray[44];

		TESTANDSET_MAXTEMP( 0, 66 );
		TESTANDSET_MAXTEMP( 0, 77 );
		TESTANDSET_MAXTEMP( 0, 88 );
        Temps[0]->max_temp = CONVERTTEMP_NCP(Temps[0]->max_temp_counts);

		sc30_temp = CONVERTTEMP_SC30(AverageTempArray[55]);
		if (sc30_temp > Temps[0]->max_temp) Temps[0]->max_temp = sc30_temp;

#ifdef DEBUG_TEMPS
		printf("IDX: 55 TEMP: %d   SC30\n", sc30_temp);
#endif

		// CPU 2
		Temps[1]->max_temp_counts = AverageTempArray[99];
		TESTANDSET_MAXTEMP( 1, 121 );
		TESTANDSET_MAXTEMP( 1, 132 );
		TESTANDSET_MAXTEMP( 1, 143 );
        Temps[1]->max_temp = CONVERTTEMP_NCP(Temps[1]->max_temp_counts);

        sc30_temp = CONVERTTEMP_SC30(AverageTempArray[110]);
        if (sc30_temp > Temps[1]->max_temp) Temps[1]->max_temp = sc30_temp;
#ifdef DEBUG_TEMPS
        printf("IDX: 110 TEMP: %d   SC30\n", sc30_temp);
#endif


		// MISC
		Temps[2]->max_temp_counts = AverageTempArray[89];
		TESTANDSET_MAXTEMP( 2, 100 );
		TESTANDSET_MAXTEMP( 2, 165 );
		Temps[2]->max_temp = CONVERTTEMP_NCP(Temps[2]->max_temp_counts);


		// RAM
		Temps[3]->max_temp_counts = AverageTempArray[1];
		TESTANDSET_MAXTEMP( 3, 2 );
		TESTANDSET_MAXTEMP( 3, 12 );
		TESTANDSET_MAXTEMP( 3, 13 );
		TESTANDSET_MAXTEMP( 3, 23 );
		TESTANDSET_MAXTEMP( 3, 24 );
		TESTANDSET_MAXTEMP( 3, 34 );
		TESTANDSET_MAXTEMP( 3, 35 );
		TESTANDSET_MAXTEMP( 3, 45 );
		TESTANDSET_MAXTEMP( 3, 46 );
		TESTANDSET_MAXTEMP( 3, 56 );
		TESTANDSET_MAXTEMP( 3, 57 );
		TESTANDSET_MAXTEMP( 3, 67 );
		TESTANDSET_MAXTEMP( 3, 68 );
		TESTANDSET_MAXTEMP( 3, 78 );
		TESTANDSET_MAXTEMP( 3, 79 );
		TESTANDSET_MAXTEMP( 3, 90 );
		TESTANDSET_MAXTEMP( 3, 101 );
		TESTANDSET_MAXTEMP( 3, 111 );
		TESTANDSET_MAXTEMP( 3, 122 );
		TESTANDSET_MAXTEMP( 3, 133 );
		TESTANDSET_MAXTEMP( 3, 144 );
		TESTANDSET_MAXTEMP( 3, 155 );
		TESTANDSET_MAXTEMP( 3, 166 );
		Temps[3]->max_temp = CONVERTTEMP_NCP(Temps[3]->max_temp_counts);
;
		// DIMM
		Temps[4]->max_temp_counts = AverageTempArray[3];
		TESTANDSET_MAXTEMP( 4, 4 );
		TESTANDSET_MAXTEMP( 4, 14 );
		TESTANDSET_MAXTEMP( 4, 15 );
		TESTANDSET_MAXTEMP( 4, 25 );
		TESTANDSET_MAXTEMP( 4, 26 );
		TESTANDSET_MAXTEMP( 4, 36 );
		TESTANDSET_MAXTEMP( 4, 37 );
		TESTANDSET_MAXTEMP( 4, 47 );
		TESTANDSET_MAXTEMP( 4, 48 );
		TESTANDSET_MAXTEMP( 4, 58 );
		TESTANDSET_MAXTEMP( 4, 59 );
		TESTANDSET_MAXTEMP( 4, 69 );
		TESTANDSET_MAXTEMP( 4, 70 );
		TESTANDSET_MAXTEMP( 4, 80 );
		TESTANDSET_MAXTEMP( 4, 81 );
		TESTANDSET_MAXTEMP( 4, 91 );
		TESTANDSET_MAXTEMP( 4, 92 );
		TESTANDSET_MAXTEMP( 4, 102 );
		TESTANDSET_MAXTEMP( 4, 103 );
		TESTANDSET_MAXTEMP( 4, 112 );
		TESTANDSET_MAXTEMP( 4, 113 );
		TESTANDSET_MAXTEMP( 4, 123 );
		TESTANDSET_MAXTEMP( 4, 124 );
		TESTANDSET_MAXTEMP( 4, 134 );
		TESTANDSET_MAXTEMP( 4, 135 );
		TESTANDSET_MAXTEMP( 4, 145 );
		TESTANDSET_MAXTEMP( 4, 146 );
		TESTANDSET_MAXTEMP( 4, 156 );
		TESTANDSET_MAXTEMP( 4, 157 );
		TESTANDSET_MAXTEMP( 4, 167 );
		TESTANDSET_MAXTEMP( 4, 168 );
        Temps[4]->max_temp = CONVERTTEMP_NCP(Temps[4]->max_temp_counts);

		// M.2
		Temps[5]->max_temp_counts = AverageTempArray[5];
		TESTANDSET_MAXTEMP( 5, 16 );
		TESTANDSET_MAXTEMP( 5, 27 );
		TESTANDSET_MAXTEMP( 5, 38 );
		TESTANDSET_MAXTEMP( 5, 49 );
		TESTANDSET_MAXTEMP( 5, 60 );
		TESTANDSET_MAXTEMP( 5, 71 );
		TESTANDSET_MAXTEMP( 5, 82 );
		TESTANDSET_MAXTEMP( 5, 93 );
		TESTANDSET_MAXTEMP( 5, 104 );
		TESTANDSET_MAXTEMP( 5, 115 );
		TESTANDSET_MAXTEMP( 5, 126 );
		TESTANDSET_MAXTEMP( 5, 137 );
		TESTANDSET_MAXTEMP( 5, 148 );
		TESTANDSET_MAXTEMP( 5, 159 );
		TESTANDSET_MAXTEMP( 5, 170 );
        Temps[5]->max_temp = CONVERTTEMP_NCP(Temps[5]->max_temp_counts);
;
		// SFF
		Temps[6]->max_temp_counts = AverageTempArray[114];
		TESTANDSET_MAXTEMP( 6, 125 );
		TESTANDSET_MAXTEMP( 6, 136 );
		TESTANDSET_MAXTEMP( 6, 147 );
        Temps[6]->max_temp = CONVERTTEMP_NCP(Temps[6]->max_temp_counts);


		// MEZZ
		Temps[7]->max_temp_counts = AverageTempArray[98];
		TESTANDSET_MAXTEMP( 7, 109 );
        Temps[7]->max_temp = CONVERTTEMP_NCP(Temps[7]->max_temp_counts);

		// Board
		Temps[8]->max_temp_counts = AverageTempArray[6];
		TESTANDSET_MAXTEMP( 8, 7 );
		TESTANDSET_MAXTEMP( 8, 17 );
		TESTANDSET_MAXTEMP( 8, 18 );
		TESTANDSET_MAXTEMP( 8, 28 );
		TESTANDSET_MAXTEMP( 8, 29 );
		TESTANDSET_MAXTEMP( 8, 39 );
		TESTANDSET_MAXTEMP( 8, 40 );
		TESTANDSET_MAXTEMP( 8, 50 );
		TESTANDSET_MAXTEMP( 8, 51 );
		TESTANDSET_MAXTEMP( 8, 61 );
		TESTANDSET_MAXTEMP( 8, 62 );
		TESTANDSET_MAXTEMP( 8, 72 );
		TESTANDSET_MAXTEMP( 8, 73 );
		TESTANDSET_MAXTEMP( 8, 83 );
		TESTANDSET_MAXTEMP( 8, 84 );
		TESTANDSET_MAXTEMP( 8, 94 );
		TESTANDSET_MAXTEMP( 8, 95 );
		TESTANDSET_MAXTEMP( 8, 97 );
		TESTANDSET_MAXTEMP( 8, 105 );
		TESTANDSET_MAXTEMP( 8, 106 );
		TESTANDSET_MAXTEMP( 8, 108 );
		TESTANDSET_MAXTEMP( 8, 116 );
		TESTANDSET_MAXTEMP( 8, 117 );
		TESTANDSET_MAXTEMP( 8, 119 );
		TESTANDSET_MAXTEMP( 8, 127 );
		TESTANDSET_MAXTEMP( 8, 128 );
		TESTANDSET_MAXTEMP( 8, 130 );
		TESTANDSET_MAXTEMP( 8, 138 );
		TESTANDSET_MAXTEMP( 8, 139 );
		TESTANDSET_MAXTEMP( 8, 141 );
		TESTANDSET_MAXTEMP( 8, 149 );
		TESTANDSET_MAXTEMP( 8, 150 );
		TESTANDSET_MAXTEMP( 8, 152 );
		TESTANDSET_MAXTEMP( 8, 160 );
		TESTANDSET_MAXTEMP( 8, 161 );
		TESTANDSET_MAXTEMP( 8, 163 );
		TESTANDSET_MAXTEMP( 8, 171 );
		TESTANDSET_MAXTEMP( 8, 172 );
		TESTANDSET_MAXTEMP( 8, 174 );
        Temps[8]->max_temp = CONVERTTEMP_NCP(Temps[8]->max_temp_counts);

		// Air
		Temps[9]->max_temp_counts = AverageTempArray[9];
		TESTANDSET_MAXTEMP( 9, 10 );
		TESTANDSET_MAXTEMP( 9, 20 );
		TESTANDSET_MAXTEMP( 9, 21 );
		TESTANDSET_MAXTEMP( 9, 31 );
		TESTANDSET_MAXTEMP( 9, 32 );
		TESTANDSET_MAXTEMP( 9, 42 );
		TESTANDSET_MAXTEMP( 9, 43 );
		TESTANDSET_MAXTEMP( 9, 53 );
		TESTANDSET_MAXTEMP( 9, 54 );
		TESTANDSET_MAXTEMP( 9, 64 );
		TESTANDSET_MAXTEMP( 9, 65 );
		TESTANDSET_MAXTEMP( 9, 75 );
		TESTANDSET_MAXTEMP( 9, 76 );
		TESTANDSET_MAXTEMP( 9, 86 );
		TESTANDSET_MAXTEMP( 9, 87 );
		TESTANDSET_MAXTEMP( 9, 96 );
		TESTANDSET_MAXTEMP( 9, 107 );
		TESTANDSET_MAXTEMP( 9, 118 );
		TESTANDSET_MAXTEMP( 9, 129 );
		TESTANDSET_MAXTEMP( 9, 140 );
		TESTANDSET_MAXTEMP( 9, 151 );
		TESTANDSET_MAXTEMP( 9, 162 );
		TESTANDSET_MAXTEMP( 9, 173 );
        Temps[9]->max_temp = CONVERTTEMP_JT(Temps[9]->max_temp_counts);

        //Find the Max Overall Temperature
        max_temp = Temps[0]->max_temp;
        for (i =0 ;i <  NUM_TEMPS; i++){
            if(Temps[i]->max_temp > max_temp)
                max_temp = Temps[i]->max_temp;
        }

		//Verify all temps are within spec.
		err = 0;
        if(Temps[0]->max_temp > RHU1_TEMP_MAX_LIMIT)
        {
            err |= 0x1;
        }

        if(Temps[1]->max_temp > RHU2_TEMP_MAX_LIMIT)
        {
            err |= 0x2;
        }

        //Remaining RHUs
	    for(i =2; i < RHU_COUNT; i++){
	        if(Temps[i]->max_temp > RHU_TEMP_MAX_LIMIT){
	            err |= ( 1 << i );
	        }
	    }

	    //Board
	    if(Temps[8]->max_temp > BOARD_TEMP_MAX_LIMIT)
	    {
	        err |= ( 1 << 8 );
	    }

	    //Air
	    if(Temps[9]->max_temp > AIR_TEMP_MAX_LIMIT)
	    {
	        err |= ( 1 << 9 );
	    }

		return err;
	}else{
	    return 0;
	}
}

//Thermistor temperature converter
uint16_t ConvertTemp_Generic(uint16_t counts_, double beta_, double r_inf_)
{
    double R         = 0;                   // measured resistance of NTC
    double tempDen   = 0;                   // denominator of temp calc
    double tempNum   = 0;                   // numerator of temp calc
    double T         = 0;                   // result of temp calc in Kelvin

    //Calculate resistance reading
    tempNum = RES_DIV*(double)(4096 - counts_);
    tempNum /= (double)4096;
    tempDen = (double)(4096 - counts_)/(double)4096;
    tempDen = (double)1 - tempDen;
    R = tempNum / tempDen;                  // measured resistance value of NTC

    tempDen = log(R / r_inf_);               // used natural log to calculate the denominator of temp calc
    T = beta_ / tempDen;                  // temp in K
    T -= C_TO_K;                            // converts to C
    T *= 10;                                // converts to C * 10
    return (uint16_t) T;                     // returns at uint16_t
}

void InitTemp(void)
{
	int i = 0;

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

