#include "led_handle.h"

// 只能刚上电时调用：
void led_init(void)
{
    led_all_off();

    last_led_gear = 0; // 赋值为初始值
    cur_led_gear_in_charging = 0;
    led_setting_mode_exit_times_cnt = 0; // 清除设置模式下的退出时间计时
    flag_is_in_struction_mode = 0;
}

void led_all_off(void)
{
    LED_1_OFF();
    LED_2_OFF();
    LED_3_OFF();
    LED_4_OFF();
    LED_5_OFF();
}

// 清除led有关的状态
void led_status_clear(void)
{
    led_all_off();

    // 清空设置模式相关的变量
    flag_is_in_setting_mode = 0;
    flag_led_setting_mode_exit_times_come = 0;
    led_setting_mode_exit_times_cnt = 0;

    // 清空指示模式相关的变量
    flag_is_in_struction_mode = 0;
    flag_led_struction_mode_exit_times_come = 0;
    led_struction_mode_exit_times_cnt = 0;

    // flag_is_led_off_enable = 0; // 不能加入这句，如果重复按下SET或红色按键，会导致无法关指示灯
}

/**
 * @brief 更改led_mode
 *
 * @param led_mode
 *              相关定义在 枚举类型 CUR_LED_MODE_XXX 中

 */
void led_mode_alter(u8 led_mode)
{
    led_all_off();

    cur_led_mode = led_mode;
}

/**
 * @brief 在设置模式，设置初始放电挡位或放电速率
 *          函数内部会判断 当前 是否处于设置模式
 *
 * @param set_led_mode
 *          CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE 要设置的是初始放电挡位
 *          CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE     要设置的是放电速率
 * @param val 初始放电挡位 或 放电速率对应的值（需要注意不能超过范围）
 */
void set_led_mode_status(u8 set_led_mode, u8 val)
{
    // 如果在设置模式，才会进入
    if (flag_is_in_setting_mode)
    {
        if (CUR_LED_MODE_INITIAL_DISCHARGE_GEAR_IN_SETTING_MODE == set_led_mode)
        {
            cur_initial_discharge_gear = val;

            // 设置了初始挡位之后，更新当前主灯光对应的占空比值
            cur_light_pwm_duty_val = light_pwm_duty_init_val_table[cur_initial_discharge_gear - 1];
        }
        else // CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE == set_led_mode
        {
            cur_discharge_rate = val;
        }

        cur_led_mode = set_led_mode;
        led_all_off();
        flag_led_setting_mode_exit_times_come = 0;
        led_setting_mode_exit_times_cnt = 0; // 清空退出设置模式的时间计数

        // led_status_clear();

        light_blink(val);
    }
}

//
void led_handle(void)
{
    // if (CUR_LED_MODE_OFF == cur_led_mode)
    // {
    //     // led_status_refresh();
    //     led_all_off();

    //     // last_led_mode = cur_led_mode;
    //     return;
    // }

    // 如果当前处于电池电量指示模式
    // 设备处于放电时，电量指示灯只显示电池电量降低的部分
    if (CUR_LED_MODE_BAT_INDICATOR == cur_led_mode ||
        CUR_LED_MODE_BAT_INDICATIOR_IN_INSTRUCTION_MODE == cur_led_mode)
    {
        /* 点亮指示灯1（放电模式，指示灯1始终点亮） */
        LED_1_ON();

        if (bat_adc_val > BAT_ADC_VAL_4) // 电池电量大于4格
        {
            cur_led_gear = 5; // 亮5格
        }
        else if (bat_adc_val > BAT_ADC_VAL_3) // 电池电量大于3格
        {
            cur_led_gear = 4; // 亮4格
        }
        else if (bat_adc_val > BAT_ADC_VAL_2) // 电池电量大于2格
        {
            cur_led_gear = 3; // 亮3格
        }
        else if (bat_adc_val > BAT_ADC_VAL_1) // 电池电量大于1格
        {
            cur_led_gear = 2; // 亮2格
        }
        else
        {
            cur_led_gear = 1; // 亮1格
        }

        if (0 == last_led_gear)
        {
            // 如果未初始化
            last_led_gear = cur_led_gear;
        }
        else
        {
            // 如果 last_led_gear 不为0，则说明已经初始化过了

            if (cur_led_gear > last_led_gear ||         //
                (0 == flag_led_gear_update_times_come)) /* 如果更新时间还未到来 */
            {
                // 如果当前要显示的指示灯 大于 上次显示的指示灯（样机在电池电压上升的情况下，不会更新显示）
                cur_led_gear = last_led_gear;
            }

            if (flag_led_gear_update_times_come)
            {
                flag_led_gear_update_times_come = 0;
            }
        }

        if (cur_led_gear >= 5)
        {
            LED_5_ON();
        }
        else
        {
            LED_5_OFF();
        }

        if (cur_led_gear >= 4)
        {
            LED_4_ON();
        }
        else
        {
            LED_4_OFF();
        }

        if (cur_led_gear >= 3)
        {
            LED_3_ON();
        }
        else
        {
            LED_3_OFF();
        }

        if (cur_led_gear >= 2)
        {
            LED_2_ON();
        }
        else
        {
            LED_2_OFF();
        }

    } // if (CUR_LED_MODE_BAT_INDICATOR == cur_led_mode)
    else if (CUR_LED_MODE_DISCHARGE_RATE_IN_SETTING_MODE == cur_led_mode ||
             CUR_LED_MODE_DISCHARGE_RATE_IN_INSTRUCTION_MODE == cur_led_mode)
    {
        // 放电速率指示模式，M1、M2、M3，需要对应的指示灯常亮
        switch (cur_discharge_rate)
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
        }
    }
    else if (CUR_LED_MODE_CHARGING == cur_led_mode) // 充电指示模式
    {
        // 在充电指示模式中，如果电池电量降低，不更新显示
        // TODO：样机是在电池电压超过3.55V并且进入涓流充电，绿色指示灯才一直点亮(第5格指示灯)

        /* 点亮指示灯1（指示灯1始终点亮） */
        LED_1_ON();

        // if (bat_adc_val > BAT_ADC_VAL_5) // 电池电量大于5格
        // {
        //     cur_led_gear = 6; // 亮5格，并且所有指示灯常亮
        // }
        // else if (bat_adc_val > BAT_ADC_VAL_4) // 电池电量大于4格
        if (bat_adc_val > BAT_ADC_VAL_4) // 电池电量大于4格
        {
            cur_led_gear = 5; // 亮5格，让第5格闪烁
        }
        else if (bat_adc_val > BAT_ADC_VAL_3) // 电池电量大于3格
        {
            cur_led_gear = 4; // 亮4格，让第4格闪烁
        }
        else if (bat_adc_val > BAT_ADC_VAL_2) // 电池电量大于2格
        {
            cur_led_gear = 3; // 亮3格，让第3格闪烁
        }
        else if (bat_adc_val > BAT_ADC_VAL_1) // 电池电量大于1格
        {
            cur_led_gear = 2; // 亮2格，让第2格闪烁
        }
        else
        {
            cur_led_gear = 1; // 亮1格，让第2格闪烁
        }

        if (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE == cur_charge_phase || /* 快满电，涓流充电 */
            CUR_CHARGE_PHASE_FULLY_CHARGE == cur_charge_phase)                                /* 已经充满电 */
        {
            // 样机是进入涓流充电才把所有指示灯变为常亮
            cur_led_gear = 6;
        }

        if (0 == last_led_gear)
        {
            last_led_gear = cur_led_gear;
        }
        else
        {
            if (cur_led_gear < last_led_gear ||         /* 如果电池电量比原来的还要低 */
                (0 == flag_led_gear_update_times_come)) /* 如果更新时间还未到来 */
            {
                // 在充电指示模式中，如果电池电量降低，不更新显示
                cur_led_gear = last_led_gear;
            }

            if (flag_led_gear_update_times_come)
            {
                flag_led_gear_update_times_come = 0;
            }
        }

        cur_led_gear_in_charging = cur_led_gear; // 更新数值，给定时器中断使用
        delay_ms(1);

        if (cur_led_gear >= 3)
        {
            LED_2_ON();
        }

        if (cur_led_gear >= 4)
        {
            LED_3_ON();
        }

        if (cur_led_gear >= 5)
        {
            LED_4_ON();
        }

        if (cur_led_gear >= 6)
        {
            LED_5_ON();
        }
    }

    // led设置模式的时间到来
    if (flag_led_setting_mode_exit_times_come)
    {
        flag_led_setting_mode_exit_times_come = 0;

        flag_is_in_setting_mode = 0; // 退出设置模式
        flag_allow_light_in_setting_mode = 0;

        // 如果要回到 led_off
        if (flag_is_led_off_enable)
        {
            flag_is_led_off_enable = 0;
            cur_led_mode = CUR_LED_MODE_OFF;
        }
        else
        {
            cur_led_mode = CUR_LED_MODE_BAT_INDICATOR; // 恢复到电池电量指示模式
        }
    }

    // led指示灯退出指示模式的时间到来
    if (flag_led_struction_mode_exit_times_come)
    {
        flag_led_struction_mode_exit_times_come = 0;

        flag_is_in_struction_mode = 0;

        // 如果要回到 led_off
        if (flag_is_led_off_enable)
        {
            flag_is_led_off_enable = 0;
            cur_led_mode = CUR_LED_MODE_OFF;
        }
        else
        {
            cur_led_mode = CUR_LED_MODE_BAT_INDICATOR;
        }
    }

    if (CUR_LED_MODE_OFF == cur_led_mode)
    {
        led_all_off();
    }

    // last_led_mode = cur_led_mode;
}
