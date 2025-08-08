#ifndef __TIMER1_H
#define __TIMER1_H

#include "include.h"

// #define PEROID_VAL (SYSCLK / 128 / 1000 - 1) // 周期值=系统时钟/分频/频率 - 1
#define TIMER1_LOW_FEQ_PEROID_VAL (SYSCLK / 1 / 20000 - 1) // 周期值=系统时钟/分频/频率 - 1

#define TIMER1_HIGH_FEQ_PEROID_VAL (SYSCLK / 1 / 104000 - 1 - 1) // 周期值=系统时钟/分频/频率 - 1（频率不准确，需要加上补偿）

void timer1_pwm_config(void);
void timer1_set_pwm_high_feq(void);
void timer1_set_pwm_low_feq(void);

void timer1_pwm_enable(void);
// 关闭pwm，引脚输出低电平
void timer1_pwm_disable(void);

#endif
