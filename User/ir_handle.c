#include "ir_handle.h"
#include "user_config.h"

void ir_handle(void)
{
    static u8 last_ir_data = 0; // 记录上一次收到的键值，在长按时使用

    if (cur_charge_phase != CUR_CHARGE_PHASE_NONE)
    {
        // 如果当前正在充电，直接返回
        flag_is_recved_data = 0;
        flag_is_recv_ir_repeat_code = 0;
        last_ir_data = 0;
        return;
    }

    if (flag_is_recved_data)
    {
        flag_is_recved_data = 0;

        last_ir_data = ir_data;

        switch (ir_data)
        {
            // =================================================================
            // 大摇控器的红色按键，小遥控器的绿色按键                             //
            // =================================================================
        case IR_KEY_RED:

            if (flag_is_in_setting_mode)
            {
                // 正处于设置模式，不响应
                return;
            }

            led_status_clear();
            // led_all_off();                         // 关闭所有led
            // flag_led_struction_mode_exit_times_come = 0;
            // led_struction_mode_exit_times_cnt = 0; // 清空led指示模式退出时间

            flag_is_in_struction_mode = 1; // 标志位置一，一段时间后退出

            if (CUR_LED_MODE_OFF == cur_led_mode)
            {
                // 如果是从关灯进入到led指示模式
                flag_is_led_off_enable = 1; // 该标志位置一，如果指示模式退出，led_mode变为led_off
            }

            if (cur_led_mode < CUR_LED_MODE_IN_INSTRUCTION_MODE) // 如果不处于指示模式
            {
                cur_led_mode = CUR_LED_MODE_BAT_INDICATIOR_IN_INSTRUCTION_MODE; // 指示模式 子模式 电池电量指示
            }
            else if (CUR_LED_MODE_BAT_INDICATOR == cur_led_mode)
            {
                // 电池电量指示模式，变为 指示模式 - 子模式 初始放电档位指示
                cur_led_mode = CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_INSTRUCTION_MODE;
            }
            else // 如果处于指示模式
            {
                cur_led_mode++;
                if (cur_led_mode > CUR_LED_MODE_DISCHARGE_RATE_IN_INSTRUCTION_MODE)
                {
                    cur_led_mode = CUR_LED_MODE_BAT_INDICATIOR_IN_INSTRUCTION_MODE;
                }
            }

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

            break;

            // =================================================================
            // 数字1                                                           //
            // =================================================================
        case IR_KEY_NUM_1:

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 1);

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

            break;

            // =================================================================
            // 亮度减，也是小遥控器的数字2                                      //
            // =================================================================
        case IR_KEY_BRIGHTNESS_SUB_OR_NUM_2:

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 2);

            // 如果不在设置模式，才调节亮度：
            if (0 == flag_is_in_setting_mode)
            {
                // 查表，找到当前亮度对应表格中的位置
                u8 i;
                for (i = 0; i < (u8)(ARRAY_SIZE(light_pwm_sub_table) - 1); i++)
                {
                    if (cur_light_pwm_duty_val > light_pwm_sub_table[i])
                    {
                        break;
                    }
                }

                cur_light_pwm_duty_val = light_pwm_sub_table[i];
                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 这个操作应该可以统一放到主函数中来更新
            }

            // printf("light pwm sub\n");

            break;

            // =================================================================
            // 3H，也是小遥控器的数字3                                          //
            // =================================================================
        case IR_KEY_3H_OR_NUM_3:

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作
            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 3);

            // 如果不在设置模式 并且 不在 指示模式，才调节
            // if (0 == flag_is_in_setting_mode && 0 == flag_is_in_struction_mode)
            if (0 == flag_is_in_setting_mode &&   /* 不在设置模式 */
                0 == flag_is_in_struction_mode && /* 不在指示模式 */
                CUR_LED_MODE_OFF != cur_led_mode) /* 未关机 */
            {
                // 设置未当前初始挡位对应的亮度：
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                // light_auto_shutdown_time_cnt = (u32)3 * 60 * 60 * 1000; // 3 小时
                light_auto_shutdown_time_cnt = (u32)3 * 1000; // 3 s，测试时使用
                flag_is_auto_shutdown_enable = 1;

                light_blink(3);
                // printf("3H press\n");
            }

            break;

            // =================================================================
            // 亮度加，也是小遥控器的数字4                                      //
            // =================================================================
        case IR_KEY_BRIGHTNESS_ADD_OR_NUM_4:

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 4);

            // 如果不在设置模式，才调节亮度：
            if (0 == flag_is_in_setting_mode)
            {
                // 查表，找到当前亮度对应表格中的位置
                u8 i;
                for (i = 0; i < ARRAY_SIZE(light_pwm_add_table) - 1; i++)
                {
                    if (cur_light_pwm_duty_val < light_pwm_add_table[i])
                    {
                        break;
                    }
                }

                // 亮度增加时，不能超过当前的初始挡位
                if (light_pwm_add_table[i] > light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1])
                {
                    cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                }
                else
                {
                    cur_light_pwm_duty_val = light_pwm_add_table[i];
                }

                flag_is_adjust_light_slowly = 0;            // 关闭缓慢调节主灯光的操作
                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 这个操作应该可以统一放到主函数中来更新
            }

            break;

            // =================================================================
            // 自动模式 ，也是小遥控器的数字5                                   //
            // =================================================================
        case IR_KEY_AUTO_OR_NUM_5:

            set_led_mode_status(CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE, 5);

            // 如果不在设置模式 并且 不在 指示模式，才调节
            if (0 == flag_is_in_setting_mode &&
                0 == flag_is_in_struction_mode &&
                CUR_LED_MODE_OFF != cur_led_mode) /* 未关机 */
            {
                flag_is_auto_shutdown_enable = 0; // 关闭定时关机的功能

                // 直接设置为当前初始挡位对应的亮度：
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                // 清空调节时间计时值：
                light_adjust_time_cnt = 0;
                LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val);
                light_blink(3);
            }

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

            break;

            // =================================================================
            // 5H，也是小遥控器的M1                                             //
            // =================================================================
        case IR_KEY_5H_OR_M1:
            // TODO：待完善功能

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE, 1);

            // 如果不在设置模式 并且 不在 指示模式，才调节
            // if (0 == flag_is_in_setting_mode && 0 == flag_is_in_struction_mode)
            if (0 == flag_is_in_setting_mode &&   /* 不在设置模式 */
                0 == flag_is_in_struction_mode && /* 不在指示模式 */
                CUR_LED_MODE_OFF != cur_led_mode) /* 未关机 */
            {
                // 直接设置为当前初始挡位对应的亮度：
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                // light_auto_shutdown_time_cnt = (u32)5 * 60 * 60 * 1000; // 5 小时
                light_auto_shutdown_time_cnt = (u32)5 * 1000; // 5 s，测试时使用
                flag_is_auto_shutdown_enable = 1;
                light_blink(3);
                // printf("5H press\n");
            }

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

            break;

            // =================================================================
            // 小遥控器的M2                                                    //
            // =================================================================
        case IR_KEY_M2:

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE, 2);

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

            break;

        case IR_KEY_8H_OR_M3:
            // 8H，也是小遥控器的M3

            set_led_mode_status(CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE, 3);

            // 如果不在设置模式 并且 不在 指示模式，才调节
            // if (0 == flag_is_in_setting_mode && 0 == flag_is_in_struction_mode)
            if (0 == flag_is_in_setting_mode &&   /* 不在设置模式 */
                0 == flag_is_in_struction_mode && /* 不在指示模式 */
                CUR_LED_MODE_OFF != cur_led_mode) /* 未关机 */
            {
                // 直接设置为当前初始挡位对应的亮度：
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                // light_auto_shutdown_time_cnt = (u32)8 * 60 * 60 * 1000; // 8 小时
                light_auto_shutdown_time_cnt = (u32)8 * 1000; // 8 s，测试时使用
                flag_is_auto_shutdown_enable = 1;
                light_blink(3);
                // printf("8H press\n");
            }

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

            break;

            // =================================================================
            // SET 模式设置                                                    //
            // =================================================================
        case IR_KEY_SET:

            flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

            if (flag_is_in_setting_mode)
            {
            }
            else // 不处于设置模式下，才进入
            {
                led_status_clear();

                if (CUR_LED_MODE_OFF == cur_led_mode)
                {
                    // 如果之前指示灯是关闭的
                    flag_allow_light_in_setting_mode = 1; // 主灯光闪烁完成后，立即关灯
                    flag_is_led_off_enable = 1;           // 设置模式退出后，关闭led指示灯

                    /*
                        如果从关灯进入设置模式，灯光亮度要回到当前挡位对应的初始亮度
                    */
                    cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                }

                cur_led_mode = CUR_LED_MODE_SETTING;
                flag_is_in_setting_mode = 1;             // 表示进入设置模式
                light_blink(cur_initial_discharge_gear); // 第一次进入设置模式，主灯光闪烁，闪烁次数对应初始放电档位
            }

            break;

            // =================================================================
            // 开灯                                                          //
            // =================================================================
        case IR_KEY_ON:

            // if (flag_is_in_setting_mode || flag_is_in_struction_mode)
            // {
            //     // 如果在设置或指示模式，不处理ON按键
            //     return;
            // }

            if (0 == flag_is_in_setting_mode && /* 不在设置模式 */
                0 == flag_is_in_struction_mode) /* 不在指示模式 */
            {
                flag_is_auto_shutdown_enable = 0; // 关闭定时关机的功能

                led_status_clear();
                flag_is_led_off_enable = 0;
                cur_led_mode = CUR_LED_MODE_BAT_INDICATOR;

                flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

                light_init();
            }

            break;

            // =================================================================
            // OFF 关灯                                                       //
            // =================================================================
        case IR_KEY_OFF:

            // if ((flag_is_in_setting_mode || flag_is_in_struction_mode) || /* 如果在设置或指示模式 */
            //     CUR_LED_MODE_OFF == cur_led_mode)                         /* 已经关机 */
            // {
            //     // 不处理OFF按键
            //     return;
            // }

            if (0 == flag_is_in_setting_mode &&   /* 不在设置模式 */
                0 == flag_is_in_struction_mode && /* 不在指示模式 */
                CUR_LED_MODE_OFF != cur_led_mode) /* 未关机 */
            {
                // 这里不能直接套用 power_off() 函数，样机的遥控器按下off后，还需要闪烁主灯光
                flag_is_adjust_light_slowly = 0;  // 关闭缓慢调节主灯光的操作
                flag_is_auto_shutdown_enable = 0; // 关闭定时关机的功能

                /* 如果灯光还是亮的 */
                /* 当前未在充电 */
                if (cur_light_pwm_duty_val > 0)
                {
                    led_status_clear();
                    cur_led_mode = CUR_LED_MODE_OFF;
                    light_blink(2);
                    cur_light_pwm_duty_val = 0; //
                }
            }

            break;

            // =================================================================
            // 全亮                                                           //
            // =================================================================
        case IR_KEY_FULL_BRIGHTNESS:

            if (0 == flag_is_in_setting_mode &&   /* 不在设置模式 */
                0 == flag_is_in_struction_mode && /* 不在指示模式 */
                CUR_LED_MODE_OFF != cur_led_mode) /* 未关机 */
            {
                flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

                // 直接设置为当前初始挡位对应的亮度：
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];

                light_blink(3);
            }

            break;

            // =================================================================
            // 半亮                                                           //
            // =================================================================
        case IR_KEY_HALF_BRIGHTNESS:

            if (0 == flag_is_in_setting_mode &&   /* 不在设置模式 */
                0 == flag_is_in_struction_mode && /* 不在指示模式 */
                CUR_LED_MODE_OFF != cur_led_mode) /* 未关机 */
            {
                flag_is_adjust_light_slowly = 0; // 关闭缓慢调节主灯光的操作

                // 设置为当前初始挡位亮度值的一半
                cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1] / 2;

                light_blink(3);
            }

            break;
        } // switch (ir_data)
    } // if (flag_is_recved_data)

    // 收到重复码
    if (flag_is_recv_ir_repeat_code)
    {
        flag_is_recv_ir_repeat_code = 0;

        /* 如果灯光还是亮的 */
        /* 当前未在充电 */
        if (cur_light_pwm_duty_val > 0)
        {
            expect_light_pwm_duty_val = cur_light_pwm_duty_val; // 获取一次当前主灯光的亮度值

            if (IR_KEY_BRIGHTNESS_ADD_OR_NUM_4 == last_ir_data)
            {
                // 亮度加 每次变化 2.59%
                if (cur_light_pwm_duty_val <
                    (light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1] -
                     (u16)((u32)TIMER2_FEQ * 259 / 10000))) /* 当前亮度值小于初始亮度值减去 2.59% */
                {
                    expect_light_pwm_duty_val += (u16)((u32)TIMER2_FEQ * 259 / 10000);
                }
                else
                {
                    /*
                        当前亮度值不小于初始亮度值减去 2.59%，直接变为初始亮度值
                    */
                    expect_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
                }

                // LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 这个操作应该可以统一放到主函数中来更新
                flag_is_adjust_light_slowly = 1; // 让定时器缓慢调节占空比
            }
            else if (IR_KEY_BRIGHTNESS_SUB_OR_NUM_2 == last_ir_data)
            {
                // 亮度减 每次变化 2.59%
                if (cur_light_pwm_duty_val >
                    (light_pwm_sub_table[ARRAY_SIZE(light_pwm_sub_table) - 1] +
                     (u16)((u32)TIMER2_FEQ * 259 / 10000))) /* 当前亮度值大于最小亮度值加 2.59% */
                {
                    expect_light_pwm_duty_val -= (u16)((u32)TIMER2_FEQ * 259 / 10000);
                }
                else
                {
                    /*
                        当前亮度值不大于最小亮度值加 2.59%，直接变为最小亮度值
                    */
                    expect_light_pwm_duty_val = light_pwm_sub_table[ARRAY_SIZE(light_pwm_sub_table) - 1];
                }

                // LIGHT_SET_PWM_DUTY(cur_light_pwm_duty_val); // 这个操作应该可以统一放到主函数中来更新
                flag_is_adjust_light_slowly = 1; // 让定时器缓慢调节占空比
            }
        }
    }
}
