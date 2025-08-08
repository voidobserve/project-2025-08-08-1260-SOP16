// encoding UTF-8
// 灯光控制源程序
#include "light_handle.h"

void light_blink(u8 blink_cnt)
{
    // u8 i;
    // for (i = 0; i < 3; i++)
    // {
    //     LIGHT_ON();
    //     delay_ms(161);
    //     LIGHT_OFF();
    //     delay_ms(161);
    // }

    light_ctl_blink_times = blink_cnt;
    flag_is_ctl_light_blink = 1; // 使能主灯光闪烁
}

void light_init(void)
{
    /* 根据初始的放电挡位来设定灯光对应的pwm占空比 */

#if 0
    switch (cur_initial_discharge_gear)
    {
    case 1:
        // 初始放电挡位 1档，刚开始是 83.67%开始放电

        // 定时器对应的重装载值最大值 对应 100%占空比
        // expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 8367 / 10000);
        cur_light_pwm_duty_val = ((u32)TIMER2_FEQ * 8367 / 10000);

        break;

    case 2:
        // 初始放电挡位 2档，刚开始是 74.11%开始放电

        // 定时器对应的重装载值最大值 对应 100%占空比
        // expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 7411 / 10000);
        cur_light_pwm_duty_val = ((u32)TIMER2_FEQ * 7411 / 10000);

        break;

    case 3:
        // 初始放电挡位 3档，刚开始是 64.55%开始放电

        // 定时器对应的重装载值最大值 对应 100%占空比
        // expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 6455 / 10000);
        cur_light_pwm_duty_val = ((u32)TIMER2_FEQ * 6455 / 10000);

        break;

    case 4:
        // 初始放电挡位 4档，刚开始是 56.98%开始放电

        // 定时器对应的重装载值最大值 对应 100%占空比
        // expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 5698 / 10000);
        cur_light_pwm_duty_val = ((u32)TIMER2_FEQ * 5698 / 10000);

        break;

    case 5:
        // 初始放电挡位 5档，刚开始是 49.8%开始放电

        // 定时器对应的重装载值最大值 对应 100%占空比
        // expect_light_pwm_duty_val = ((u32)TIMER2_FEQ * 4980 / 10000);
        cur_light_pwm_duty_val = ((u32)TIMER2_FEQ * 4980 / 10000);

        break;
    } 
    
    // cur_light_pwm_duty_val = expect_light_pwm_duty_val;
#endif

    // 查表，获得挡位对应的占空比值
    cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];

    LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 立刻更新PWM占空比
    LIGHT_ON();                                 // 使能PWM输出
    light_blink(3);                             // 开机前，主灯需要闪烁
    light_adjust_time_cnt = 0;                  // 灯光调整时间清零
}

/**
 * @brief 灯光控制（放电控制）
 *          进入前要先确认 expect_light_pwm_duty_val 的值是否初始化过一次，
 *          进入前要先确认 cur_light_pwm_duty_val 的值是否初始化过一次，
 *          light_adjust_time_cnt调节灯光的时间计时是否正确，如果切换了模式或放电速度，要先清零
 */
void light_handle(void)
{
    // if (flag_is_need_to_exit_setting_mode_close_light && 0 == flag_is_in_setting_mode)
    // {
    //     LIGHT_OFF();
    //     flag_is_need_to_exit_setting_mode_close_light = 0;
    // }

    // 如果正在充电，直接返回
    if (cur_charge_phase != CUR_CHARGE_PHASE_NONE ||
        cur_led_mode == CUR_LED_MODE_OFF) /* 如果指示灯已经关闭 */
    {
        return;
    }

    // 如果未在充电

    // if (flag_is_light_adjust_time_come)
    // {
    //     flag_is_light_adjust_time_come = 0;
    //     light_adjust_time_cnt++;
    // }

    if (1 == cur_discharge_rate) // 放电速率1档，M1
    {
        /*
            速度为M1，
            1200s后变化一次占空比，(1200 * 1)
            3600s后再变化一次，    (1200 * 3)
            7200s后再变化一次，    (1200 * 6)
            ...
            假设之后是：
            (1200 * 9)
            (1200 * 12)
            (1200 * 15)
            ...
            每次变化约10%占空比
        */

        if (light_adjust_time_cnt >= (1200 * light_ctl_phase_in_rate_1)) // 如果到了调节时间
        {
            light_adjust_time_cnt = 0;

            if (1 == light_ctl_phase_in_rate_1)
            {
                light_ctl_phase_in_rate_1 = 3;
            }
            else
            {
                light_ctl_phase_in_rate_1 += 3;
            }

            // 定时器对应的重装载值最大值 对应 100%占空比
            // if (expect_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 10 / 100))
            // {
            //     // 如果仍大于 4.8% + 10%， 减少10%占空比
            //     expect_light_pwm_duty_val -= (u32)TIMER2_FEQ * 10 / 100;
            // }
            // else
            // {
            //     // 4.8%占空比
            //     expect_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
            // }
            if (cur_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 10 / 100))
            {
                // 如果仍大于 4.8% + 10%， 减少10%占空比
                cur_light_pwm_duty_val -= (u32)TIMER2_FEQ * 10 / 100;
            }
            else
            {
                // 4.8%占空比
                cur_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
            }
        }
    }
    else // 2 == cur_discharge_rate || 3 == cur_discharge_rate
    {
        /*
            一开始每40s降低一次占空比
            从47%开始，每240s降低一次占空比
            从42%开始，每420s降低一次占空比

            暂定每次降低 0.6%
        */

        // 当前的占空比在47%以上时，不包括47%，每40s降低一次占空比
        if (cur_light_pwm_duty_val > (u32)TIMER2_FEQ * 47 / 100)
        {
            if (light_adjust_time_cnt >= 40)
            {
                light_adjust_time_cnt = 0;

                // if (expect_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                // {
                //     // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                //     expect_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                // }
                // else
                // {
                //     // 4.8%占空比
                //     expect_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                // }
                if (cur_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                {
                    // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                    cur_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                }
                else
                {
                    // 4.8%占空比
                    cur_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                }
            }
        }
        // 当前的占空比在42%以上时，不包括42%，每240秒降低一次占空比
        else if (cur_light_pwm_duty_val > (u32)TIMER2_FEQ * 42 / 100)
        {
            if (light_adjust_time_cnt >= 240)
            {
                light_adjust_time_cnt = 0;

                // if (expect_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                // {
                //     // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                //     expect_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                // }
                // else
                // {
                //     // 4.8%占空比
                //     expect_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                // }
                if (cur_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                {
                    // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                    cur_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                }
                else
                {
                    // 4.8%占空比
                    cur_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                }
            }
        }
        else // 当前的占空比在42%及以下，每420秒降低一次占空比
        {
            if (light_adjust_time_cnt >= 420)
            {
                light_adjust_time_cnt = 0;

                // if (expect_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                // {
                //     // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                //     expect_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                // }
                // else
                // {
                //     // 4.8%占空比
                //     expect_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                // }
                if (cur_light_pwm_duty_val >= ((u32)TIMER2_FEQ * 48 / 1000) + ((u32)TIMER2_FEQ * 6 / 1000))
                {
                    // 如果仍大于 4.8% + xx %， 减少 xx %占空比
                    cur_light_pwm_duty_val -= (u32)TIMER2_FEQ * 6 / 1000;
                }
                else
                {
                    // 4.8%占空比
                    cur_light_pwm_duty_val = (u32)TIMER2_FEQ * 48 / 1000;
                }
            }
        }
    } // 放电速率M2，放电速率M3

    // // 如果缓慢调节PWM占空比的时间到来 -- 需要放到定时器中断调节，主循环的时间过长
    // if (flag_is_light_pwm_duty_val_adjust_time_come)
    // {
    //     flag_is_light_pwm_duty_val_adjust_time_come = 0;

    //     if (cur_light_pwm_duty_val > expect_light_pwm_duty_val)
    //     {
    //         // 如果要调小灯光的占空比
    //         cur_light_pwm_duty_val--;
    //     }
    //     else if (cur_light_pwm_duty_val < expect_light_pwm_duty_val)
    //     {
    //         // 如果要调大灯光的占空比
    //         cur_light_pwm_duty_val++;
    //     }

    //     timer2_set_pwm_duty(cur_light_pwm_duty_val);
    // }
    LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val);
}

// void light_set_pwm_duty(u16 pwm_duty_val)
// {
//     timer2_set_pwm_duty(pwm_duty_val);
// }
