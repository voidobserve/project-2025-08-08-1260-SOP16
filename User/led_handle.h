#ifndef __LED_HANDLE_H
#define __LED_HANDLE_H

#include "include.h"
#include "user_config.h"

/*
    3.2V以上，（1）（2）（3）（4）（5）点亮
    3.05V以上，（1）（2）（3）（4）
    2.85V以上，（1）（2）（3）
    2.65V以上，（1）（2）
    2.65V以下，（1）
*/

// 指示灯从左往右数,分为1~5
// 电池电压检测脚检测到的电压,为电池的1/2分压
// 定义电池各个电压下对应的AD值:
enum
{
    BAT_ADC_VAL_1 = (u16)((u32)2650 * 4096 / 2 / 2 / 1000), /* 2713.6 -- 电池2.65V对应的ad值 */
    BAT_ADC_VAL_2 = (u16)((u32)2850 * 4096 / 2 / 2 / 1000),
    BAT_ADC_VAL_3 = (u16)((u32)3050 * 4096 / 2 / 2 / 1000),
    BAT_ADC_VAL_4 = (u16)((u32)3200 * 4096 / 2 / 2 / 1000),

    BAT_ADC_VAL_5 = (u16)((u32)3400 * 4096 / 2 / 2 / 1000),
};

// #define BAT_ADC_VAL_DEAD_ZONE (50) // 电池电压对应的ad值死区

enum
{
    CUR_LED_MODE_OFF = 0, // 关机，指示灯全灭

    CUR_LED_MODE_BAT_INDICATOR, // 电池电量指示模式

    // CUR_LED_MODE_INITIAL_DISCHARGE_GEAR, // 初始放电挡位 -- 从 xx% PWW开始放电（指示灯由定时器控制）
    // CUR_LED_MODE_DISCHARGE_RATE,         // 放电速率

    CUR_LED_MODE_CHARGING, // 充电指示模式

    CUR_LED_MODE_SETTING, // 刚用遥控器按下SET按键，未按下其他按键，5个指示灯会一起闪烁（指示灯由定时器控制）

    CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, // 设置模式，子模式 初始放电挡位
    CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE,    // 设置模式，子模式 放电速率

    CUR_LED_MODE_IN_INSTRUCTION_MODE, // 指示模式，只用作比较，不用作赋值

    CUR_LED_MODE_BAT_INDICATIOR_IN_INSTRUCTION_MODE,         // 指示模式，子模式 电池电量指示
    CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_INSTRUCTION_MODE, // 指示模式，子模式 初始放电档位指示
    CUR_LED_MODE_DISCHARGE_RATE_IN_INSTRUCTION_MODE,         // 指示模式，子模式 放电速率指示

    CUR_LED_MODE_MAX = 0xFF,
};

// #define BAT_ADC_VAL_SAMPLE_COUNT 20 // 滑动平均的样本计数

// extern volatile u8 bat_remaining_power_indication; // 电池剩余电量指示挡位

void led_init(void);
void led_all_off(void);

// void led_status_refresh(void);
// 清除led有关的状态
void led_status_clear(void);

void led_mode_alter(u8 led_mode);
void led_handle(void);

void set_led_mode_status(u8 set_led_mode, u8 val);

#endif
