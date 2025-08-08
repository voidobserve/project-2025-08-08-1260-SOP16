#ifndef __CHARGE_HANDLE_H
#define __CHARGE_HANDLE_H

#include "include.h"
#include "user_config.h"

// 定义当前充电的状态
// enum
// {
//     CUR_CHARGE_STATUS_NONE = 0x00, // 未在充电
//     CUR_CHARGE_STATUS_IN_CHARGING, // 正在充电


//     // CUR_CHARGE_STATUS_PRE_CHARGING, // 准备进入充电
//     // CUR_CHARGE_STATUS_TRICKLE_CHARGE_WHEN_BAT_IS_LOW,       // 电池电量低，进行涓流充电
//     // CUR_CHARGE_STATUS_CHARGE_NORMALLY,                      // 电池正常充电
//     // CUR_CHARGE_STATUS_TRICKLE_CHARGE_WHEN_BAT_IS_NEAR_FULL, // 电池快满电，进行涓流充电
// };

// 定义当前充电阶段
enum
{
    CUR_CHARGE_PHASE_NONE,
    CUR_CHARGE_PHASE_TRICKLE_CHARGE, // 电池电量低，涓流充电
    CUR_CHARGE_PHASE_NORMAL_CHARGE, // 电池电量正常，恒功率充电
    CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE, // 接近满电，进行涓流充电
    CUR_CHARGE_PHASE_FULLY_CHARGE, // 满电，此时需要等充电端断电
};

// 定义当前控制充电的PWM状态
enum
{
    CUR_CHARGING_PWM_STATUS_NONE = 0x00,
    CUR_CHARGING_PWM_STATUS_LOW_FEQ,  // pwm 低频率
    CUR_CHARGING_PWM_STATUS_HIGH_FRQ, // pwm 高频率
};

// 控制充电的pwm设置为高频率，用于正常充电
#define PWM_CTL_FOR_CHARGING_SET_HIGH_FEQ() \
    do                                      \
    {                                       \
        timer1_set_pwm_high_feq();          \
    } while (0);

// 控制充电的pwm设置为低频率，用于涓流充电
#define PWM_CTL_FOR_CHARGING_SET_LOW_FEQ() \
    do                                     \
    {                                      \
        tiemr1_set_pwm_low_feq();          \
    } while (0);

// 关闭控制充电的pwm，对应引脚输出低电平
#define PWM_CTL_FOR_CHARGING_DISABLE() \
    do                                 \
    {                                  \
        timer1_pwm_disable();          \
    } while (0);

// #define PWM_CTL_FOR_CHARGING_REG

// extern volatile u8 cur_charge_status;

void charge_handle(void);

#endif
