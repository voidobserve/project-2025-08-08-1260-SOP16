
#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

#include "include.h"
#include <stdio.h>

#include "uart0.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"
#include "adc.h"
#include "charge_handle.h"
#include "led_handle.h"
#include "ir_handle.h"
#include "light_handle.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define USE_MY_DEBUG 0

// =================================================================
// 红外接收相关变量                                                //
// =================================================================
extern volatile u8 ir_data;
extern volatile bit flag_is_recv_ir_repeat_code;
extern volatile bit flag_is_recved_data;

// =================================================================
// 充电控制相关变量                                                 //
// =================================================================
extern volatile u16 bat_adc_val;                      // 电池电压检测脚采集到的ad值
extern volatile u16 charging_adc_val;                 // 充电电压检测脚采集的ad值
extern volatile u16 current_adc_val;                  // 充电电流检测脚采集的ad值
extern volatile u8 flag_is_charging_adjust_time_come; // 调节充电的时间到来
extern volatile u8 cur_charging_pwm_status;           // 控制充电的PWM状态
extern volatile u8 cur_charge_phase;                  // 记录当前充电阶段

// =================================================================
// 指示灯控制相关变量                                               //
// =================================================================
extern volatile u8 cur_initial_discharge_gear; // 初始放电挡位（需要记忆）
extern volatile u8 cur_discharge_rate;         // 初始放电速率（需要记忆）
extern volatile u8 cur_led_mode;               // 当前的LED模式
// extern volatile u8 last_led_mode;              // 上次的led模式
extern volatile u8 cur_led_gear;             // 当前led挡位
extern volatile u8 last_led_gear;            // 上次led挡位
extern volatile u8 cur_led_gear_in_charging; // 充电指示，对应的挡位
extern volatile bit flag_is_in_setting_mode; // 是否处于设置模式

extern volatile u16 led_setting_mode_exit_times_cnt;      // 特殊的LED模式，退出时间计数
extern volatile u8 flag_led_setting_mode_exit_times_come; // 标志位，led退出设置模式的时间到来

extern volatile bit flag_is_in_struction_mode;               // 是否退出指示灯指示模式
extern volatile bit flag_led_struction_mode_exit_times_come; // 退出指示灯指示模式的时间到来
extern volatile u16 led_struction_mode_exit_times_cnt;       // 退出指示灯指示模式时间计数

extern volatile bit flag_led_gear_update_times_come; // 指示灯状态更新的时间到来
// extern volatile u16 led_gear_update_times_cnt ; // 指示灯状态更新的时间计数

// 标志位，是否要回到 led_off 模式
extern volatile bit flag_is_led_off_enable;

// =================================================================
// 主灯光控制相关变量                                               //
// =================================================================
extern volatile u32 light_adjust_time_cnt;    // 调节灯光的时间计数，暂定为每1s加一
extern volatile u8 light_ctl_phase_in_rate_1; // 在放电速率M1时，使用到的变量，在计算公式里面用作系数，每次唤醒时需要初始化为1

// TODO：3260使用16位寄存器，7361使用8位寄存器，要进行适配修改
extern volatile u16 cur_light_pwm_duty_val; // 当前灯光对应的占空比值
// extern volatile u16 expect_light_pwm_duty_val;                  // 期望调节到的、灯光对应的占空比值
extern volatile u8 flag_is_light_adjust_time_come;              // 调节灯光的时间到来，目前为1s
extern volatile u8 flag_is_light_pwm_duty_val_adjust_time_come; // 灯光占空比值调节时间到来

extern volatile u8 flag_is_ctl_light_blink; // 是否控制主灯光闪烁
extern volatile u8 light_ctl_blink_times;   // 要控制主灯光闪烁的次数
/*
    是否要在设置模式期间关闭主灯光

    如果已经关灯，在设置模式期间，主灯闪烁完成后，直接关灯
*/
extern volatile bit flag_allow_light_in_setting_mode;

// 是否要缓慢调节主灯光的占空比
extern volatile bit flag_is_adjust_light_slowly;
extern volatile u16 expect_light_pwm_duty_val; // 期望缓慢调节到的、主灯光对应的占空比值

// 是否开启了定时关机功能：
extern volatile bit flag_is_auto_shutdown_enable;
extern volatile u32 light_auto_shutdown_time_cnt;     // 定时关机功能的定时器计数，单位：ms
extern volatile bit flag_is_auto_shutdown_times_come; // 定时关机的时间到来

extern const u16 light_pwm_sub_table[9];
extern const u16 light_pwm_add_table[9];
extern const u16 light_pwm_duty_init_val_table[5];

enum
{
    ADC_REF_2_0_VOL = 0x00, // adc使用2.0V参考电压
    ADC_REF_3_0_VOL,        // adc使用3.0V参考电压
};

#define LED_1_PIN P00
#define LED_2_PIN P05
#define LED_3_PIN P06
#define LED_4_PIN P13
#define LED_5_PIN P14

// LED点亮或关闭时，对应的引脚电平
#define LED_ON_LEVEL 1
#define LED_OFF_LEVEL 0

// typedef union
// {
//     unsigned char byte;
//     struct
//     {
//         u8 bit0 : 1;
//         u8 bit1 : 1;
//         u8 bit2 : 1;
//         u8 bit3 : 1;
//         u8 bit4 : 1;
//         u8 bit5 : 1;
//         u8 bit6 : 1;
//         u8 bit7 : 1;
//     } bits;
// } bit_flag;
// extern volatile bit_flag flag1;
// extern volatile bit_flag flag2;

// #define flag_is_led_1_enable flag2.bits.bit0 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
// #define flag_is_led_2_enable flag2.bits.bit1 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
// #define flag_is_led_3_enable flag2.bits.bit2 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
// #define flag_is_led_4_enable flag2.bits.bit3 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
// #define flag_is_led_5_enable flag2.bits.bit4 // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮

extern volatile bit flag_is_led_1_enable;
extern volatile bit flag_is_led_2_enable;
extern volatile bit flag_is_led_3_enable;
extern volatile bit flag_is_led_4_enable;
extern volatile bit flag_is_led_5_enable;

#define LED_1_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_1_enable = 1; \
        } while (0);                  \
    }
#define LED_1_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_1_enable = 0; \
        } while (0);                  \
    }
#define LED_2_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_2_enable = 1; \
        } while (0);                  \
    }
#define LED_2_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_2_enable = 0; \
        } while (0);                  \
    }
#define LED_3_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_3_enable = 1; \
        } while (0);                  \
    }
#define LED_3_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_3_enable = 0; \
        } while (0);                  \
    }
#define LED_4_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_4_enable = 1; \
        } while (0);                  \
    }
#define LED_4_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_4_enable = 0; \
        } while (0);                  \
    }
#define LED_5_ON()                    \
    {                                 \
        do                            \
        {                             \
            flag_is_led_5_enable = 1; \
        } while (0);                  \
    }
#define LED_5_OFF()                   \
    {                                 \
        do                            \
        {                             \
            flag_is_led_5_enable = 0; \
        } while (0);                  \
    }


    
extern void power_off(void);


#endif