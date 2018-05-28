/*
 * temps.c
 *
 *  Created on: Mar 9, 2018
 *      Author: Zack Lyzen
 */

#include "temps.h"
#include "ntd_debug.h"

#define NUM_TEMPS 		10

struct temps_perRHU_{
	uint16_t 	max_temp;                           //max temp in xxx format
	uint16_t    flagged;                            //RHU is flagged as overheat
};

static struct temps_perRHU_ Temps[NUM_TEMPS];       


uint32_t *temps_inCounts;
uint16_t temps_bySensor[176] = {0};

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
    switch(rhu_){
    case 0: return temps_bySensor[55];
    case 1: return temps_bySensor[110];
    case 2:{
            //MISCIC, return the larger of sensor 89 or 100
            return ( temps_bySensor[89] > temps_bySensor[100] ) ? temps_bySensor[89] : temps_bySensor[100];
        }
    }

    return Temps[rhu_].max_temp;
}

uint16_t TEMPS_GetFlag( uint16_t rhu_ ){
    return Temps[rhu_].flagged;
}

#define TESTANDSET_MAXTEMP( arr_, idx_, MAX_, temp_ )\
        temps_bySensor[idx_] = temp_;\
        if ( temps_bySensor[idx_] > Temps[arr_].max_temp ) Temps[arr_].max_temp = temps_bySensor[idx_];\
        if ( temps_bySensor[idx_] > MAX_ ) { Temps[arr_].flagged = 1; flagged++; }

#define TESTANDSET_MAXTEMP_NCP( arr_, idx_, MAX_)\
            TESTANDSET_MAXTEMP( arr_, idx_, MAX_, CONVERTTEMP_NCP(temps_inCounts[idx_]) )

#define TESTANDSET_MAXTEMP_JT( arr_, idx_, MAX_)\
            TESTANDSET_MAXTEMP( arr_, idx_, MAX_, CONVERTTEMP_JT(temps_inCounts[idx_]) )

#define TESTANDSET_MAXTEMP_INIT( arr_, idx_, MAX_, temp_ )\
        Temps[arr_].flagged = 0;\
        Temps[arr_].max_temp = temps_bySensor[idx_] = temp_;\
        if ( temps_bySensor[idx_] > MAX_ ) { Temps[arr_].flagged = 1; flagged++; }

#define TESTANDSET_MAXTEMP_NCP_INIT( arr_, idx_, MAX_)\
            TESTANDSET_MAXTEMP_INIT( arr_, idx_, MAX_, CONVERTTEMP_NCP(temps_inCounts[idx_]) )

#define TESTANDSET_MAXTEMP_JT_INIT( arr_, idx_, MAX_)\
            TESTANDSET_MAXTEMP_INIT( arr_, idx_, MAX_, CONVERTTEMP_JT(temps_inCounts[idx_]) )

/*
 * RETURNS
 *
 * -1 = NOT READY
 * 0 = TEMPS GOOD
 * >0 OTHER, bitfield of relevant RHUs that are out of spec as defined below
 *
 * CPU1                        0x1
 * CPU2                        0x2
 * MISC                        0x4
 * RAM                         0x8
 * DIMM_GRP                    0x10
 * M_2_GRP                     0x20
 * SFF_GRP                     0x40
 * MEZZ                        0x80
 * BOARD                       0x100
 * AIR                         0x200
 *
 *
 *#define   AIR_TEMP_MAX_LIMIT          700         // temp value (c * 10) before error is thrown
#define     BOARD_TEMP_MAX_LIMIT        700         // Board temp limit
#define     RHU1_TEMP_MAX_LIMIT         900         // CPU1
#define     RHU2_TEMP_MAX_LIMIT         900         // CPU2
#define     RHU_TEMP_MAX_LIMIT          700         // All other RHUs
*/

int ProcessTempData(void)
{
    /*NTD - VERIFIED 05/10/18. ALL IDX ACCURATE*/

    int err = 0, i=0, flagged = 0;

    if( !GetNtcReady() ) return 0;

    ClearNtcReady();
    temps_inCounts = GetTempData();

    // CPU 1
    TESTANDSET_MAXTEMP_NCP_INIT( 0, 44, MAXTEMP_KAPTONHTR );        //THRM1-CPU1
    TESTANDSET_MAXTEMP_NCP( 0, 66, MAXTEMP_VRM );                   //VRM1-T1
    TESTANDSET_MAXTEMP_NCP( 0, 77, MAXTEMP_VRM);                    //VRM1-T3
    TESTANDSET_MAXTEMP_NCP( 0, 88, MAXTEMP_VRM);                    //VRM1-T3

    temps_bySensor[55] = CONVERTTEMP_SC30(temps_inCounts[55]);      //THRM2-CPU1
    if (temps_bySensor[55] > Temps[0].max_temp) Temps[0].max_temp = temps_bySensor[55];
    if (temps_bySensor[55] > MAXTEMP_CPU ) Temps[0].flagged = 1;

    // CPU 2
    TESTANDSET_MAXTEMP_NCP_INIT( 1, 99, MAXTEMP_KAPTONHTR );        //THRM1-CPU2
    TESTANDSET_MAXTEMP_NCP( 1, 121, MAXTEMP_VRM );                  //VRM2-T1
    TESTANDSET_MAXTEMP_NCP( 1, 132, MAXTEMP_VRM );                  //VRM2-T2
    TESTANDSET_MAXTEMP_NCP( 1, 143, MAXTEMP_VRM );                  //VRM2-T3

    temps_bySensor[110] = CONVERTTEMP_SC30(temps_inCounts[110]);    //THRM2-CPU1
    if (temps_bySensor[110] > Temps[1].max_temp) Temps[1].max_temp = temps_bySensor[110];
    if (temps_bySensor[110] > MAXTEMP_CPU ) Temps[1].flagged = 1;

    // MISC
    TESTANDSET_MAXTEMP_NCP_INIT( 2, 89, MAXTEMP_MISC );             //T-IC2
    TESTANDSET_MAXTEMP_NCP( 2, 100, MAXTEMP_MISC );                 //T-IC1
    TESTANDSET_MAXTEMP_NCP( 2, 165, MAXTEMP_KAPTONHTR );            //CSETSIM_THRM1

    // RAM
    TESTANDSET_MAXTEMP_NCP_INIT( 3, 1, MAXTEMP_RAM);
    TESTANDSET_MAXTEMP_NCP( 3, 2, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 12, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 13, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 23, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 24, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 34, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 35, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 45, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 46, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 56, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 57, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 67, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 68, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 78, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 79, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 90, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 101, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 111, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 122, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 133, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 144, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 155, MAXTEMP_RAM );
    TESTANDSET_MAXTEMP_NCP( 3, 166, MAXTEMP_RAM );

    // DIMM
    TESTANDSET_MAXTEMP_NCP_INIT( 4, 3, MAXTEMP_DIMM );
    TESTANDSET_MAXTEMP_NCP( 4, 4, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 14, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 15, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 25, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 26, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 36, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 37, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 47, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 48, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 58, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 59, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 69, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 70, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 80, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 81, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 91, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 92, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 102, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 103, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 112, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 113, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 123, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 124, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 134, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 135, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 145, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 146, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 156, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 157, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 167, MAXTEMP_DIMM  );
    TESTANDSET_MAXTEMP_NCP( 4, 168, MAXTEMP_DIMM  );

    // M.2
    TESTANDSET_MAXTEMP_NCP_INIT( 5, 5, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 16, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 27, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 38, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 49, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 60, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 71, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 82, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 93, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 104, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 115, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 126, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 137, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 148, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 159, MAXTEMP_M2 );
    TESTANDSET_MAXTEMP_NCP( 5, 170, MAXTEMP_M2 );

    // SFF
    TESTANDSET_MAXTEMP_NCP_INIT( 6, 114, MAXTEMP_SFF );
    TESTANDSET_MAXTEMP_NCP( 6, 125, MAXTEMP_SFF );
    TESTANDSET_MAXTEMP_NCP( 6, 136, MAXTEMP_SFF );
    TESTANDSET_MAXTEMP_NCP( 6, 147, MAXTEMP_SFF );

    // MEZZ
    TESTANDSET_MAXTEMP_NCP_INIT( 7, 98, MAXTEMP_MEZZ );
    TESTANDSET_MAXTEMP_NCP( 7, 109, MAXTEMP_MEZZ );

    // Board
    TESTANDSET_MAXTEMP_NCP_INIT( 8, 6, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 7, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 17, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 18, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 28, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 29, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 39, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 40, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 50, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 51, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 61, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 62, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 72, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 73, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 83, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 84, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 94, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 95, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 97, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 105, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 106, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 108, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 116, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 117, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 119, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 127, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 128, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 130, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 138, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 139, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 141, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 149, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 150, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 152, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 160, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 161, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 163, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_NCP( 8, 171, MAXTEMP_MISC );
    //TESTANDSET_MAXTEMP_NCP( 8, 172, MAXTEMP_MISC );   //disabled as this thermistor is a: not cooled, and b: adjacent to CPU1
    TESTANDSET_MAXTEMP_NCP( 8, 174, MAXTEMP_MISC );

    // Air
#ifndef DISABLE_AIR
    TESTANDSET_MAXTEMP_JT_INIT( 9, 9, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 10, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 20, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 21, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 31, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 32, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 42, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 43, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 53, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 54, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 64, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 65, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 75, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 76, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 86, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 87, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 96, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 107, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 118, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 129, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 140, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 151, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 162, MAXTEMP_MISC );
    TESTANDSET_MAXTEMP_JT( 9, 173, MAXTEMP_MISC );
#else
    Temps[9].flagged = 0;
    Temps[9].max_temp = 0;
#endif

    //Find the Max Overall Temperature
    max_temp = Temps[0].max_temp;
    for (i =0 ;i <  NUM_TEMPS; i++){
        if(Temps[i].max_temp > max_temp)
            max_temp = Temps[i].max_temp;
    }

    //Verify all temps are within spec.
    err = 0;
    for(i =0; i < NUM_TEMPS; i++){
        if(Temps[i].flagged){
            #ifdef DEBUG_TEMPS
            switch (i){
            case 0: { printf("ProcessTemps():: Flagging CPU1 with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 1: { printf("ProcessTemps():: Flagging CPU2 with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 2: { printf("ProcessTemps():: Flagging MISC with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 3: { printf("ProcessTemps():: Flagging RAM with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 4: { printf("ProcessTemps():: Flagging DIMM_GRP with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 5: { printf("ProcessTemps():: Flagging M2_GRP with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 6: { printf("ProcessTemps():: Flagging SFF_GRP with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 7: { printf("ProcessTemps():: Flagging MEZZ with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 8: { printf("ProcessTemps():: Flagging BOARD with Overtemp = %d\n", Temps[i].max_temp); break; }
            case 9: { printf("ProcessTemps():: Flagging AIR with Overtemp = %d\n", Temps[i].max_temp); break; }
            }
            #endif
            err += ( 1 << i );
        }
    }

    #ifdef DEBUG_TEMPS
    printf("ProcessTemps():: Returning err="PRINTF_BINSTR16"\n", PRINTF_BINSTR16_ARGS(err));

    if (err){
        for (i =0; i < 176; i++){
            printf("ProcessTemps():: IDX %d, temp = %d\n", i, temps_bySensor[i] );
        }
    }
    #endif

    return err;

}

//Thermistor temperature converter
uint16_t ConvertTemp_Generic(uint16_t counts_, double beta_, double r_inf_)
{
    double R         = 0;                   // measured resistance of NTC
    double T         = 0;                   // result of temp calc in Kelvin

    //Measurement outside of range (disconnected)
    if (counts_ == 4095 || counts_ <= 100) return 0;

    //Calculate Resistance reading
    R = 10000 * ( (4096/ (double) counts_) -1 );

    T = beta_ / log(R / r_inf_);            // temp in K
    T -= C_TO_K;                            // converts to C
    T *= 10;                                // converts to C * 10
    return (uint16_t) T;
}

void InitTemp(void)
{

}

