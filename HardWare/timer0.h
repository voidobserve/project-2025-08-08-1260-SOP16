#ifndef __TIMER0_H
#define __TIMER0_H

#include "include.h"

#define TIMER0_PERIOD_VAL (SYSCLK / 128 / 10000 - 1) // 周期值=系统时钟/分频/频率 - 1



void timer0_config(void);

#endif

