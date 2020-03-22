#ifndef TIMER_H
#define TIMER_H
#include "global.h"

extern volatile u32 timerCount;

void TIM3_Init(u16 arr,u16 psc);
void ADC_TIM2_Init(u16 arr,u16 psc);
u32 getCurrentTimerCount(void);
u32 getDelayTimerCount(u32 startTimerCount);
void mainWhileCountPerSec(void);
#endif
