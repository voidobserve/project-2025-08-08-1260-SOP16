#include "timer0.h"

#include "ir_handle.h"
#include "user_config.h"

void timer0_config(void)
{
    __EnableIRQ(TMR0_IRQn); // 使能timer0中断
    IE_EA = 1;              // 使能总中断

    // 设置timer0的计数功能，配置一个频率为1kHz的中断
    TMR_ALLCON = TMR0_CNT_CLR(0x1);                               // 清除计数值
    TMR0_PRH = TMR_PERIOD_VAL_H((TIMER0_PERIOD_VAL >> 8) & 0xFF); // 周期值
    TMR0_PRL = TMR_PERIOD_VAL_L((TIMER0_PERIOD_VAL >> 0) & 0xFF);
    TMR0_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                          // 计数等于周期时允许发生中断
    TMR0_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x7) | TMR_MODE_SEL(0x1); // 选择系统时钟，128分频，计数模式
}

void TIMR0_IRQHandler(void) interrupt TMR0_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR0_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (TMR0_CONH & TMR_PRD_PND(0x1))
    {
        TMR0_CONH |= TMR_PRD_PND(0x1); // 清除pending

#if 1     // 红外解码【需要放到100us的定时器中断来处理】
        { // 红外解码
            // static volatile u8 ir_fliter;
            static volatile u16 ir_level_cnt; // 红外信号的下降沿时间间隔计数
            static volatile u32 __ir_data;    //
            static volatile bit last_level_in_ir_pin = 0;
            // static volatile u16 ir_long_press_cnt; // 檢測紅外遙控長按的計數值

            // 对每个下降沿进行计数
            if (ir_level_cnt <= 1300)
                ir_level_cnt++;

            // 滤波操作
            // ir_fliter <<= 1;
            // if (IR_RECV_PIN)
            // {
            //     ir_fliter |= 1;
            // }
            // ir_fliter &= 0x07;

            // if (ir_fliter == 0x07)
            //     filter_level = 1;
            // else if (ir_fliter == 0x00)
            //     filter_level = 0;

            // if (filter_level)
            if (IR_RECV_PIN)
            {
                last_level_in_ir_pin = 1; // 表示接收到的是高电平

                // 如果之前也是高电平
                if (ir_level_cnt > 1200) // 超时时间(120000us,120ms)
                {
                    // if (__ir_data != 0) // 超时，且接收到数据(红外接收处理函数中会把ir_data清零)
                    if (__ir_data != 0) // 超时，且接收到数据(现在是在中断服务函数中把__ir_data自动清零)
                    {
                        // // 带校验的版本：
                        // if ((u8)(__ir_data >> 8) == (u8)(~__ir_data)) // 如果键值的原码和反码相等
                        // {
                        // flag_is_recved_data = 1;
                        // }

                        // 不带校验的版本
                        if (0 == flag_is_recved_data)
                        {
                            // if ((__ir_data & 0xFF0000) == 0xFF0000)
                            {
                                ir_data = ~__ir_data;
                                __ir_data = 0;
                                flag_is_recved_data = 1;
                            }
                        }
                    }

                    flag_is_recv_ir_repeat_code = 0; // 认为遥控器按键已经按下，然后松开
                }
            }
            else
            {
                if (last_level_in_ir_pin)
                {
                    // 如果之前检测到的是高电平，现在检测到了低电平
                    if (ir_level_cnt <= 8) // 小于800us，说明接收到无效的数据，重新接收
                    {
                        // FLAG_IS_RECVED_ERR_IR_DATA = 1;
                        flag_is_recv_ir_repeat_code = 0;
                    }
                    else if (ir_level_cnt <= 18) // 小于1800us,说明接收到了逻辑0
                    {
                        __ir_data <<= 1;

                        // P15D = 0; // 测试红外解码
                        // P15D = ~P15D; // 测试红外解码
                    }
                    else if (ir_level_cnt <= 26) // 小于2600us,说明接收到了逻辑1
                    {
                        __ir_data <<= 1;
                        __ir_data |= 0x01;

                        // P15D = 1; // 测试红外解码
                    }
                    else if (ir_level_cnt <= 150) // 小于15000us,说明接收到了格式头
                    {
                        // FLAG_IS_RECVED_ERR_IR_DATA = 1;
                        // ir_long_press_cnt = 0; // 加上这一条会检测不到长按
                    }
                    else if (ir_level_cnt <= 420) // 小于42ms,说明接收完一次完整的红外信号
                    {
#if 0 // 带校验的版本，命令源码与命令反码进行校验
    
                if ((u8)(__ir_data >> 8) == (u8)(~__ir_data)) // 如果键值的原码和反码相等
                {
                    flag_is_recved_data = 1;
                    flag_is_recv_ir_repeat_code = 1; //
                    ir_long_press_cnt = 0;
                }

#else // 不带校验的版本

                        if (0 == flag_is_recved_data) // 如果之前未接收到数据/接收到的数据已经处理完毕
                        {
                            // if ((__ir_data & 0xFF0000) == 0xFF0000)
                            {
                                ir_data = ~__ir_data;
                                __ir_data = 0;
                                flag_is_recved_data = 1;
                                // flag_is_recv_ir_repeat_code = 1; //
                            }
                        }

#endif // 不带校验的版本
                    }
                    else if (ir_level_cnt <= 1200) // 小于120000,120ms,说明接收到了重复码
                    {
                        // if (ir_long_press_cnt < 65535)
                        //     ir_long_press_cnt++;

                        flag_is_recv_ir_repeat_code = 1;
                    }
                    // else // 超过120000,说明接收到无效的数据
                    // {
                    // }

                    ir_level_cnt = 0;
                }

                last_level_in_ir_pin = 0; // 表示接收到的是低电平
            }
        } // 红外解码
#endif // 红外解码【需要放到100us的定时器中断来处理】

#if 1 // 控制LED指示灯 (软件PWM驱动)

        {                   // 控制LED指示灯--需要放在100us的中断
            static u16 cnt; // 用软件实现PWM驱动LED的相关变量
            cnt++;

            if (cnt <= 20) // 20 * 100us
            {
                if (flag_is_led_1_enable)
                {
                    LED_1_PIN = LED_ON_LEVEL;
                }

                if (flag_is_led_2_enable)
                {
                    LED_2_PIN = LED_ON_LEVEL;
                }

                if (flag_is_led_3_enable)
                {
                    LED_3_PIN = LED_ON_LEVEL;
                }

                if (flag_is_led_4_enable)
                {
                    LED_4_PIN = LED_ON_LEVEL;
                }

                if (flag_is_led_5_enable)
                {
                    LED_5_PIN = LED_ON_LEVEL;
                }
            }
            else
            {
                LED_1_PIN = LED_OFF_LEVEL;
                LED_2_PIN = LED_OFF_LEVEL;
                LED_3_PIN = LED_OFF_LEVEL;
                LED_4_PIN = LED_OFF_LEVEL;
                LED_5_PIN = LED_OFF_LEVEL;
            }

            if (cnt >= 100) // 100 * 100us
            {
                cnt = 0;
            }

        } // 控制LED指示灯--需要放在100us的中断

#endif // 控制LED指示灯(软件PWM驱动)

        {
            static u8 cnt_100us = 0;
            cnt_100us++;

            if (cnt_100us >= 10) // 10 * 100us == 1ms
            {
                cnt_100us = 0;

#if 1 // 控制LED指示灯的闪烁效果

                {
                    static u8 blink_cnt = 0;
                    static bit flag_blink_dir = 0;
                    blink_cnt++;
                    if (blink_cnt >= 200)
                    {
                        blink_cnt = 0;

                        // 处于初始放电挡位指示模式，才进入
                        if (CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE == cur_led_mode ||
                            CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_INSTRUCTION_MODE == cur_led_mode)
                        {
                            if (0 == flag_blink_dir)
                            {
                                switch (cur_initial_discharge_gear)
                                {
                                case 1:
                                    LED_1_ON();
                                    break;
                                case 2:
                                    LED_2_ON();
                                    break;
                                case 3:
                                    LED_3_ON();
                                    break;
                                case 4:
                                    LED_4_ON();
                                    break;
                                case 5:
                                    LED_5_ON();
                                    break;
                                }

                                flag_blink_dir = 1;
                            }
                            else
                            {
                                switch (cur_initial_discharge_gear)
                                {
                                case 1:
                                    LED_1_OFF();
                                    break;
                                case 2:
                                    LED_2_OFF();
                                    break;
                                case 3:
                                    LED_3_OFF();
                                    break;
                                case 4:
                                    LED_4_OFF();
                                    break;
                                case 5:
                                    LED_5_OFF();
                                    break;
                                }

                                flag_blink_dir = 0;
                            }
                        } // if (CUR_LED_MODE_INITIAL_DISCHARGE_GEAR == cur_led_mode)
                        else if (CUR_LED_MODE_SETTING == cur_led_mode)
                        {
                            // 刚按下遥控器的 SET 按键，会进入 设置模式，5个指示灯一起闪烁
                            if (0 == flag_blink_dir)
                            {
                                LED_1_ON();
                                LED_2_ON();
                                LED_3_ON();
                                LED_4_ON();
                                LED_5_ON();
                                flag_blink_dir = 1;
                            }
                            else
                            {
                                LED_1_OFF();
                                LED_2_OFF();
                                LED_3_OFF();
                                LED_4_OFF();
                                LED_5_OFF();
                                flag_blink_dir = 0;
                            }
                        }
                        // 指示灯处于充电指示模式
                        else if (CUR_LED_MODE_CHARGING == cur_led_mode)
                        {
                            if (cur_led_gear_in_charging <= 2)
                            {
                                if (0 == flag_blink_dir)
                                {
                                    LED_2_ON();
                                    flag_blink_dir = 1;
                                }
                                else
                                {
                                    LED_2_OFF();
                                    flag_blink_dir = 0;
                                }
                            }
                            else if (cur_led_gear_in_charging <= 3)
                            {
                                if (0 == flag_blink_dir)
                                {
                                    LED_3_ON();
                                    flag_blink_dir = 1;
                                }
                                else
                                {
                                    LED_3_OFF();
                                    flag_blink_dir = 0;
                                }
                            }
                            else if (cur_led_gear_in_charging <= 4)
                            {
                                if (0 == flag_blink_dir)
                                {
                                    LED_4_ON();
                                    flag_blink_dir = 1;
                                }
                                else
                                {
                                    LED_4_OFF();
                                    flag_blink_dir = 0;
                                }
                            }
                            else if (cur_led_gear_in_charging <= 5)
                            {
                                if (0 == flag_blink_dir)
                                {
                                    LED_5_ON();
                                    flag_blink_dir = 1;
                                }
                                else
                                {
                                    LED_5_OFF();
                                    flag_blink_dir = 0;
                                }
                            }
                        }

                    } // if (blink_cnt >= 200)
                }

#endif // 控制LED指示灯的闪烁效果

#if 1 // 退出led设置模式的时间计数

                // if (CUR_LED_MODE_INITIAL_DISCHARGE_GEAR == cur_led_mode ||
                //     CUR_LED_MODE_DISCHARGE_RATE == cur_led_mode ||
                //     CUR_LED_MODE_SETTING == cur_led_mode)

                if (flag_is_in_setting_mode) // 处于设置模式，开始累计时间
                {
                    // special_led_mode_times_cnt++;
                    // if (special_led_mode_times_cnt >= 5000)
                    // {
                    //     special_led_mode_times_cnt = 0;
                    //     flag_led_exit_setting_times_come = 1;
                    // }
                    led_setting_mode_exit_times_cnt++;
                    if (led_setting_mode_exit_times_cnt >= 5000)
                    {
                        led_setting_mode_exit_times_cnt = 0;
                        flag_led_setting_mode_exit_times_come = 1;
                    }
                }

#endif // 退出led设置模式的时间计数

                {
                    static u8 cnt = 0;
                    // static u16 cnt = 0;
                    cnt++;
                    if (cnt >= 200)
                    // if (cnt >= 1000) // 延长时间并不会影响电池快满电后，调节PWM的大小，还是会过冲，电流过大
                    {
                        cnt = 0;
                        flag_is_charging_adjust_time_come = 1; // 调节充电电流/功率
                    }
                }

#if 1 // 在涓流充电时，负责每段时间断开一会PWM输出
                {
                    static u16 cnt = 0;

                    if (CUR_CHARGING_PWM_STATUS_LOW_FEQ == cur_charging_pwm_status)
                    {
                        // 如果在涓流充电
                        cnt++;

                        if (cnt < 22000) // 22 s
                        {
                            timer1_pwm_enable();
                            // flag_is_tim_turn_off_pwm = 0;
                        }
                        else if (cnt <= (22000 + 60)) // 22s + 60ms
                        {
                            // 累计涓流充电22s后，关闭控制充电的PWM，之后可以在这期间检测电池是否满电
                            timer1_pwm_disable();
                            // flag_is_tim_turn_off_pwm = 1;
                            // cnt = 0;
                        }
                        else
                        {
                            cnt = 0;
                        }
                    }
                    else
                    {
                        cnt = 0;
                    }
                }
#endif // 在涓流充电时，负责每段时间断开一会PWM输出

#if 1 // 控制主灯光的闪烁效果

                {
                    static u8 blink_cnt = 0; // 记录闪烁次数
                    static u16 time_cnt = 0; // 控制灯光闪烁的时间

                    if (flag_is_ctl_light_blink)
                    {
                        blink_cnt = light_ctl_blink_times;
                        time_cnt = 0;
                        // 闪烁完成后，清除标志位
                        flag_is_ctl_light_blink = 0;
                    }

                    if (blink_cnt)
                    {
                        time_cnt++;
                        if (time_cnt < (u16)161) // 0 ~ 161 ms
                        {
                            // 如果当前在放电：
                            if (cur_led_mode != CUR_LED_MODE_OFF &&      /* 指示灯开启 */
                                cur_led_mode != CUR_LED_MODE_CHARGING && /* 不在充电 */
                                (0 == flag_allow_light_in_setting_mode)) /* 不需要闪完灯后关闭主灯光 */
                            // if (cur_led_mode != CUR_LED_MODE_OFF &&
                            //     cur_led_mode != CUR_LED_MODE_CHARGING)
                            {
                                // LIGHT_OFF();

                                LIGHT_ON();
                            }
                            else
                            {
                                // LIGHT_ON();

                                LIGHT_OFF();
                            }
                        }
                        else if (time_cnt < (u16)(161 * 2)) // 161 ~ 322 ms
                        {
                            // 如果当前在放电：
                            if (cur_led_mode != CUR_LED_MODE_OFF &&
                                cur_led_mode != CUR_LED_MODE_CHARGING &&
                                (0 == flag_allow_light_in_setting_mode))
                            // if (cur_led_mode != CUR_LED_MODE_OFF &&
                            //     cur_led_mode != CUR_LED_MODE_CHARGING)
                            {
                                // LIGHT_ON();

                                LIGHT_OFF();
                            }
                            else
                            {
                                // LIGHT_OFF();

                                LIGHT_ON();
                            }
                        }
                        else // 超过 161 * 2ms，清除时间计数，闪烁次数减一，表示完成一次闪烁
                        {
                            time_cnt = 0;
                            blink_cnt--;

                            // 如果当前在放电：
                            if (cur_led_mode != CUR_LED_MODE_OFF &&
                                cur_led_mode != CUR_LED_MODE_CHARGING &&
                                (0 == flag_allow_light_in_setting_mode))
                            // if (cur_led_mode != CUR_LED_MODE_OFF &&
                            //     cur_led_mode != CUR_LED_MODE_CHARGING)
                            {
                                LIGHT_ON();
                            }
                            else
                            {
                                LIGHT_OFF();
                            }

                            // flag_is_need_to_exit_setting_mode_close_light = 0;
                        }
                    }
                }

#endif // 控制主灯光的闪烁效果

#if 1 // 控制退出指示灯指示模式

                if (flag_is_in_struction_mode)
                {
                    led_struction_mode_exit_times_cnt++;
                    if (led_struction_mode_exit_times_cnt >= 5000)
                    {
                        led_struction_mode_exit_times_cnt = 0;
                        // flag_is_in_struction_mode = 0;
                        flag_led_struction_mode_exit_times_come = 1;
                    }
                }

#endif // 控制退出指示灯指示模式

#if 1 // 控制定时关机

                {
                    static u32 cnt = 0;

                    if (flag_is_auto_shutdown_enable)
                    {
                        cnt++; // 累计定时关机时间
                        if (cnt >= light_auto_shutdown_time_cnt)
                        {
                            // 让主函数关机
                            flag_is_auto_shutdown_times_come = 1;
                        }
                    }
                    else
                    {
                        cnt = 0;
                        flag_is_auto_shutdown_times_come = 0;
                    }
                }

#endif // 控制定时关机

#if 1
                {
                    static u16 cnt = 0;

                    if (0 == flag_led_gear_update_times_come)
                    {
                        cnt++;
                        if (cnt >= 60000)
                        {
                            cnt = 0;
                            flag_led_gear_update_times_come = 1;
                        }
                    }
                }
#endif

            } // if (cnt >= 10) // 10 * 100us == 1ms
        }

#if 1 // 放电时间控制

        // 如果不在充电，设备没有关机，才进行放电时间累计
        {
            static u16 cnt = 0;
            if (CUR_CHARGE_PHASE_NONE == cur_charge_phase && /* 如果不在充电 */
                cur_led_mode != CUR_LED_MODE_OFF)            /* 如果指示灯没有关闭 */
            {
                cnt++;
                if (cnt >= 10000) // 10000 * 100us，1s
                {
                    cnt = 0;
                    // flag_is_light_adjust_time_come = 1;

                    if (light_adjust_time_cnt < 4294967295) // 防止计数溢出
                    {
                        light_adjust_time_cnt++;
                    }
                }
            }
            else
            {
                cnt = 0;
            }
        }

#endif // 放电时间控制

#if 1 // 缓慢调节驱动灯光的pwm占空比

        {
            static u8 cnt = 0;

            // 目前是每100us调节一次

            if (flag_is_adjust_light_slowly)
            {
                cnt++;
                if (cnt >= 100) // 看起来调节效果较平滑
                {
                    cnt = 0;
                    if (cur_light_pwm_duty_val > expect_light_pwm_duty_val)
                    {
                        // 如果要调小灯光的占空比
                        cur_light_pwm_duty_val--;
                    }
                    else if (cur_light_pwm_duty_val < expect_light_pwm_duty_val)
                    {
                        // 如果要调大灯光的占空比
                        cur_light_pwm_duty_val++;
                    }
                    else
                    {
                        // 如果灯光的占空比已经达到期望值，则取消灯光的调光
                        flag_is_adjust_light_slowly = 0; //
                    }

                    LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val);
                }
            }
            // else
            // {
            //     cnt = 0;
            // }
        }

#endif // 缓慢调节驱动灯光的pwm占空比
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR0_IRQn);
}