/**
 ******************************************************************************
 * @file    main.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    01-05-2021
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 * 版权说明后续补上
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "user_config.h"
#include <stdio.h>

volatile bit flag_is_led_1_enable; // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
volatile bit flag_is_led_2_enable; // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
volatile bit flag_is_led_3_enable; // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
volatile bit flag_is_led_4_enable; // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮
volatile bit flag_is_led_5_enable; // led  是否使能，0--不使能，led 熄灭，1--使能，led 点亮

// =================================================================
// 红外接收相关变量                                                //
// =================================================================
volatile u8 ir_data = 0;
volatile bit flag_is_recv_ir_repeat_code = 0;
volatile bit flag_is_recved_data = 0;

// =================================================================
// 充电控制相关变量                                                 //
// =================================================================
volatile u16 bat_adc_val;                                           // 电池电压检测脚采集到的ad值
volatile u16 charging_adc_val;                                      // 充电电压检测脚采集的ad值
volatile u16 current_adc_val;                                       // 充电电流检测脚采集的ad值
volatile u8 flag_is_charging_adjust_time_come = 0;                  // 调节充电的时间到来
volatile u8 cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE; // 控制充电的PWM状态
volatile u8 cur_charge_phase = CUR_CHARGE_PHASE_NONE;               // 记录当前充电阶段

// =================================================================
// 指示灯控制相关变量                                               //
// =================================================================
volatile u8 cur_initial_discharge_gear; // 初始放电挡位（需要记忆）
volatile u8 cur_discharge_rate;         // 初始放电速率（需要记忆）
volatile u8 cur_led_mode;               // 当前的LED模式
// volatile u8 last_led_mode; // 上次的led模式
volatile u8 cur_led_gear;             // 当前led挡位
volatile u8 last_led_gear;            // 上次led挡位（只能在刚上电时清零赋初始值）
volatile u8 cur_led_gear_in_charging; // 充电指示，对应的挡位

volatile bit flag_is_in_setting_mode = 0;              // 是否处于设置模式
volatile u8 flag_led_setting_mode_exit_times_come = 0; // 标志位，led退出设置模式的时间到来
volatile u16 led_setting_mode_exit_times_cnt = 0;      // 特殊的LED模式，退出时间计数

volatile bit flag_is_in_struction_mode = 0;               // 是否处于指示模式
volatile bit flag_led_struction_mode_exit_times_come = 0; // 退出指示灯指示模式的时间到来
volatile u16 led_struction_mode_exit_times_cnt = 0;       // 退出指示灯指示模式时间计数

volatile bit flag_led_gear_update_times_come = 0; // 指示灯状态更新的时间到来
// volatile u16 led_gear_update_times_cnt = 0; // 指示灯状态更新的时间计数

// 标志位，是否要回到 led_off 模式
volatile bit flag_is_led_off_enable = 0;

// =================================================================
// 主灯光控制相关变量                                               //
// =================================================================
volatile u32 light_adjust_time_cnt = 0;    // 调节灯光的时间计数，暂定为每1s加一
volatile u8 light_ctl_phase_in_rate_1 = 1; // 在放电速率M1时，使用到的变量，在计算公式里面用作系数，每次唤醒时需要初始化为1

// TODO：3260使用16位寄存器，7361使用8位寄存器，要进行适配修改
volatile u16 cur_light_pwm_duty_val = 0; // 当前灯光对应的占空比值
// volatile u16 expect_light_pwm_duty_val = 0;                  // 期望调节到的、灯光对应的占空比值
volatile u8 flag_is_light_adjust_time_come = 0;              // 调节灯光的时间到来，目前为1s
volatile u8 flag_is_light_pwm_duty_val_adjust_time_come = 0; // 灯光占空比值调节时间到来

volatile u8 flag_is_ctl_light_blink = 0; // 是否控制主灯光闪烁
volatile u8 light_ctl_blink_times = 0;   // 要控制主灯光闪烁的次数
/*
    是否要在设置模式期间关闭主灯光

    如果已经关灯，在设置模式期间，主灯闪烁完成后，直接关灯
*/
volatile bit flag_allow_light_in_setting_mode = 0;

// 是否要缓慢调节主灯光的占空比
volatile bit flag_is_adjust_light_slowly = 0;
volatile u16 expect_light_pwm_duty_val = 0; // 期望缓慢调节到的、主灯光对应的占空比值

// 是否开启了定时关机功能：
volatile bit flag_is_auto_shutdown_enable = 0;
volatile u32 light_auto_shutdown_time_cnt = 0;     // 定时关机功能的定时器计数，单位：ms
volatile bit flag_is_auto_shutdown_times_come = 0; // 标志位，定时关机的时间是否到来，0--定时关机的时间未到来，1--定时关机的时间已经到来

// 短按减小灯光亮度，对应各个挡位亮度的占空比值
const u16 light_pwm_sub_table[9] = {
    (u16)((u32)TIMER2_FEQ * 8367 / 10000), // 83.67 %
    (u16)((u32)TIMER2_FEQ * 7371 / 10000), // 73.71 %
    (u16)((u32)TIMER2_FEQ * 6375 / 10000), // 63.75 %
    (u16)((u32)TIMER2_FEQ * 5379 / 10000), // 53.79 %
    (u16)((u32)TIMER2_FEQ * 4383 / 10000), // 43.83 %
    (u16)((u32)TIMER2_FEQ * 3387 / 10000), // 33.87 %
    (u16)((u32)TIMER2_FEQ * 2391 / 10000), // 23.91 %
    (u16)((u32)TIMER2_FEQ * 1395 / 10000), // 13.95 %
    (u16)((u32)TIMER2_FEQ * 478 / 10000),  // 4.78 %
};

// 短按增加灯光亮度，对应各个挡位亮度的占空比值
const u16 light_pwm_add_table[9] = {
    (u16)((u32)TIMER2_FEQ * 478 / 10000),  // 4.78 %
    (u16)((u32)TIMER2_FEQ * 1474 / 10000), // 14.74 %
    (u16)((u32)TIMER2_FEQ * 2470 / 10000), // 24.70 %
    (u16)((u32)TIMER2_FEQ * 3466 / 10000), // 34.66 %
    (u16)((u32)TIMER2_FEQ * 4462 / 10000), // 44.62 %
    (u16)((u32)TIMER2_FEQ * 5458 / 10000), // 54.58 %
    (u16)((u32)TIMER2_FEQ * 6554 / 10000), // 65.54 %
    (u16)((u32)TIMER2_FEQ * 7450 / 10000), // 74.50 %
    (u16)((u32)TIMER2_FEQ * 8367 / 10000), // 83.67 %
};

const u16 light_pwm_duty_init_val_table[5] = {
    (u16)((u32)TIMER2_FEQ * 8367 / 10000), // 83.67 %
    (u16)((u32)TIMER2_FEQ * 7411 / 10000), // 74.11 %
    (u16)((u32)TIMER2_FEQ * 6455 / 10000), // 64.55 %
    (u16)((u32)TIMER2_FEQ * 5698 / 10000), // 56.98 %
    (u16)((u32)TIMER2_FEQ * 4980 / 10000), // 49.80 %
};

void led_pin_config(void)
{
    P0_MD0 &= ~GPIO_P00_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P00_MODE_SEL(0x01); // 输出模式
    FOUT_S00 = GPIO_FOUT_AF_FUNC;      // 选择AF功能输出
    P00 = 0;                           // 如果不给初始值，上电之后，指示灯会闪一下

    P0_MD1 &= ~GPIO_P05_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P05_MODE_SEL(0x01); // 输出模式
    FOUT_S05 = GPIO_FOUT_AF_FUNC;
    P05 = 0; // 如果不给初始值，上电之后，指示灯会闪一下

    P0_MD1 &= ~GPIO_P06_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P06_MODE_SEL(0x01); // 输出模式
    FOUT_S06 = GPIO_FOUT_AF_FUNC;
    P06 = 0; // 如果不给初始值，上电之后，指示灯会闪一下

    P1_MD0 &= ~GPIO_P13_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P13_MODE_SEL(0x01); // 输出模式
    FOUT_S13 = GPIO_FOUT_AF_FUNC;
    P13 = 0; // 如果不给初始值，上电之后，指示灯会闪一下

    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P14_MODE_SEL(0x01); // 输出模式
    FOUT_S14 = GPIO_FOUT_AF_FUNC;
    P14 = 0; // 如果不给初始值，上电之后，指示灯会闪一下
}

/*
    变量、参数，初始化

    如果是第一次上电，需要读出存放的数据
*/
void param_init(void)
{
    light_ctl_phase_in_rate_1 = 1;

    cur_initial_discharge_gear = 5; // 初始放电挡位（需要记忆）
    cur_discharge_rate = 2;         // 初始放电速率（需要记忆）
}

// 关机，最后让指示灯熄灭，主灯光熄灭，准备进入低功耗
void power_off(void)
{
    flag_is_adjust_light_slowly = 0;      // 关闭缓慢调节主灯光的操作
    flag_is_auto_shutdown_enable = 0;     // 不使能自动关机
    flag_is_auto_shutdown_times_come = 0; // 清除定时关机标志
    led_status_clear();
    cur_led_mode = CUR_LED_MODE_OFF;
    cur_light_pwm_duty_val = 0;
    LIGHT_OFF();
}

void user_init(void)
{
    uart0_config();

    timer0_config();
    timer1_pwm_config(); // 控制充电的PWM
    timer1_pwm_disable();
    timer2_pwm_config(); // 控制灯光的pwm
    timer2_pwm_disable();

    led_pin_config(); // led指示灯

    // 红外接收引脚：
    // P2_MD0 &= ~(GPIO_P23_MODE_SEL(0x03)); // 输入模式
    // P2_PU |= GPIO_P23_PULL_UP(0x01);      // 上拉

    P0_PU |= GPIO_P02_PULL_UP(0x01);    // 上拉
    P0_MD0 &= ~GPIO_P02_MODE_SEL(0x03); // 输入模式

    adc_config();
    pga_config();

    // 上电后，需要先点亮红色指示灯，再变为电池电量指示模式
    LED_1_ON();
    delay_ms(1000);

    param_init();

    light_init();
    led_init();
    led_mode_alter(CUR_LED_MODE_BAT_INDICATOR); // 电池电量指示模式

    delay_ms(1); // 等待系统稳定
}

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
void main(void)
{
    // 看门狗默认打开, 复位时间2s
    WDT_KEY = WDT_KEY_VAL(0xDD); //  关闭看门狗 (如需配置看门狗请查看“WDT\WDT_Reset”示例)

    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;  // 写一个无效的数据，触发写保护

    user_init();

    // printf("sys reset\n");

    while (1)
    {
        // low_energy();

#if 1

        /* 在放电时，检测电池电压，一旦低于2.5V，直接关机 */
        if (cur_charge_phase == CUR_CHARGE_PHASE_NONE || /* 如果不在充电 */
            cur_led_mode != CUR_LED_MODE_OFF)            /* 如果led指示灯还未关闭 */
        {
            /*
                如果进入该条件，可能正在放电，检测电池电压，一旦低于2.5V，直接关机
            */
            adc_update_bat_adc_val();
            // 电池电压检测脚检测到的电压，是电池的1/2分压，adc使用2V参考电压
            if (bat_adc_val < (u16)((u32)2500 * 4096 / 2 / 2 / 1000))
            {
                power_off();
            }
        }

        charge_handle();
        ir_handle(); // 函数内部会判断是否在充电，如果在充电则退出

        /*
            【非充电模式】 -> 【充电模式】

            如果当前正在充电，但是指示灯没有切换到充电指示模式，则切换：
        */
        if (CUR_CHARGE_PHASE_NONE != cur_charge_phase)
        {
            // if (cur_led_mode != CUR_LED_MODE_CHARGING && /* 指示灯不处于充电模式 */
            //     cur_led_mode != CUR_LED_MODE_OFF)
            if (cur_led_mode != CUR_LED_MODE_CHARGING) /* 指示灯不处于充电模式 */
            {
                // 清空定时关机相关的变量
                flag_is_auto_shutdown_enable = 0;
                led_status_clear();
                led_mode_alter(CUR_LED_MODE_CHARGING);
            }

            // 需要关闭主灯光
            LIGHT_OFF();
        } // if (CUR_CHARGE_PHASE_NONE != cur_charge_phase)
        else // CUR_CHARGE_PHASE_NONE == cur_charge_phase
        {
            /*
                【充电模式】 -> 【放电、点亮主灯光、指示灯对应电池电量指示】
                如果当前没有在充电，并且指示灯处于充电指示模式，
                切换回电池电量指示模式

                测试时发现从充电到断开充电，led指示灯还在闪烁，需要加上这补丁
            */
            if (cur_led_mode == CUR_LED_MODE_CHARGING)
            {
                led_status_clear();
                led_mode_alter(CUR_LED_MODE_BAT_INDICATOR);
                // 需要打开主灯光

                // 查表，获得挡位对应的占空比值
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];

                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 立刻更新PWM占空比
                LIGHT_ON();                                 // 使能 PWM 输出
            }
        }

        // 如果定时关机的时间到来
        if (flag_is_auto_shutdown_times_come)
        {
            // flag_is_auto_shutdown_times_come = 0; // 清空定时关机标志
            // flag_is_auto_shutdown_enable = 0;     // 不允许自动关机
            // led_status_clear();
            // cur_led_mode = CUR_LED_MODE_OFF;
            // cur_light_pwm_duty_val = 0;
            // LIGHT_OFF();
            power_off();
            // printf("power off\n");
        }

        adc_update_bat_adc_val();
        led_handle();
        light_handle();

#if 0  // 每隔一段时间，打印调试信息：

        {
            static u8 cnt = 0;
            cnt++;
            if (cnt >= 200)
            {
                cnt = 0;
                // printf("bat_adc_val %u\n", bat_adc_val);
                // printf("cur_light_pwm_duty_val %u\n", cur_light_pwm_duty_val);
                // printf("cur light pwm percent %bu %%\n", (u8)((u32)cur_light_pwm_duty_val * 100 / TIMER2_FEQ));

                switch (cur_led_mode)
                {
                case CUR_LED_MODE_OFF:
                    printf("led mode off\n");
                    break;

                case CUR_LED_MODE_BAT_INDICATOR:
                    printf("led mode bat indicator\n");
                    break;

                case CUR_LED_MODE_CHARGING:
                    printf("led mode charging\n");
                    break;

                case CUR_LED_MODE_SETTING:
                    printf("led mode setting\n");
                    break;

                case CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE:
                    printf("led mode initial discharge gear in setting mode\n");
                    break;

                case CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE:
                    printf("led mode discharge rate in setting mode\n");
                    break;

                case CUR_LED_MODE_BAT_INDICATIOR_IN_INSTRUCTION_MODE:
                    printf("led mode bat indicator in instruction mode\n");
                    break;

                case CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_INSTRUCTION_MODE:
                    printf("led mode initial discharge gear in instruction mode\n");
                    break;

                case CUR_LED_MODE_DISCHARGE_RATE_IN_INSTRUCTION_MODE:
                    printf("led mode discharge rate in instruction mode\n");
                    break;

                default:
                    break;
                }
            }
        }
#endif // 每隔一段时间，打印调试信息

#if 0  // demo板使用到的调试指示灯
        if (CUR_CHARGE_PHASE_NONE == cur_charge_phase)
        {
            my_debug_led_2_off();
            my_debug_led_3_off();
            my_debug_led_4_off();

            my_debug_led_1_on();
        }
        else if (CUR_CHARGE_PHASE_NORMAL_CHARGE == cur_charge_phase)
        {
            my_debug_led_1_off();
            my_debug_led_3_off();
            my_debug_led_4_off();

            my_debug_led_2_on();
        }
        else if (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE == cur_charge_phase)
        {
            my_debug_led_1_off();
            my_debug_led_2_off();
            my_debug_led_4_off();

            my_debug_led_3_on();
        }
        else if (CUR_CHARGE_PHASE_FULLY_CHARGE == cur_charge_phase)
        {
            my_debug_led_1_off();
            my_debug_led_2_off();
            my_debug_led_3_off();

            my_debug_led_4_on();
        }
#endif // demo板使用到的调试指示灯

#endif
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
