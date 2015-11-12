#ifndef __TIMER_H__YN
#define __TIMER_H__YN

#include "DrvTimer.h"
#include "DrvGPIO.h"
#include "led.h"
#include "icSystem.h"
#include "play.h"

void icTimerInit(void);
void icTimerTerm(void);
void TimerInit(void);
void CommonCheck(void);

int16_t GetSysTicks(void);
int16_t GetWaitForRecTicks(void);
int16_t get_low_ticks(void);

#endif
