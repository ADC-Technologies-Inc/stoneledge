#ifndef HW_REALTIMECLOCK_H_
#define HW_REALTIMECLOCK_H_

typedef struct _realTimeClock RealTimeClock;

extern void InitializeRealTimeClocks(void);
extern void UnLatchRealTimeClock(RealTimeClock* realTimeClock);
extern char IsRealTimeClockLatched(RealTimeClock* realTimeClock);


#endif /* HW_REALTIMECLOCK_H_ */
