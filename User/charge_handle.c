#include "charge_handle.h"

// volatile u8 cur_charge_status = CUR_CHARGE_STATUS_NONE;

// #include "my_debug.h"

#define MIN_PWM_DUTY_IN_LOW_POWER (8)      // 快满电而降低充电功率时，最低的占空比，单位：1%
#define MIN_PWM_DUTY_IN_TRICKLE_CHARGE (5) // 涓流充电时，最低的占空比，单位：1%

// 充电控制
void charge_handle(void)
{
    if (flag_is_charging_adjust_time_come) // 一定要加入缓慢调节，不能迅速调节，否则会等不到电压稳定
    {
        static u8 pwm_duty = 0; // 单位：1%

        // 检测到电池快满电，是否进入涓流充电的计数
        static u8 trickle_charge_cnt = 0;
        // static u8 limited_pwm_duty = 0;

        u16 current = 0;        // 充电电流，单位：mA
        u16 voltage_of_bat = 0; // 电池电压，单位：mV
        u32 power = 0;          // 功率，单位：mW 毫瓦
        u16 pwm_reg = 0;        // 存放要写入到寄存器中的占空比值
        u16 expected_power = 0; // 期望功率 ，单位：mW 毫瓦

        flag_is_charging_adjust_time_come = 0; // 清除标志位

        // 如果当前没有在充电
        if (CUR_CHARGE_PHASE_NONE == cur_charge_phase)
        {
            // adc_sel_ref_voltage(ADC_REF_2_0_VOL); // 2V参考电压
            // adc_sel_pin(ADC_PIN_DETECT_CHARGE);
            // charging_adc_val = adc_getval();
            adc_update_charge_adc_val(ADC_REF_2_0_VOL);

            // 如果充电输入电压大于4.9V，使能充电
            if (charging_adc_val >= (u16)((u32)4900 * 4096 / 11 / 2 / 1000))
            {
                cur_charge_phase = CUR_CHARGE_PHASE_TRICKLE_CHARGE; // 刚进入充电，默认是电池电量低对应的涓流充电
            }

            // adc_sel_pin(ADC_PIN_DETECT_BATTERY);
            // bat_adc_val = adc_getval();
            // if (bat_adc_val >= (u16)((u32)(3600 - 50) * 4096 / 2 / 2 / 1000))
            // {
            //     // 检测到电池电压大于3.6 - 0.05V
            //     // 不使能充电
            //     cur_charge_phase = CUR_CHARGE_PHASE_NONE;
            // }

            // 如果充电电压未满足使能充电的条件，会进入下面的语句块
            if (CUR_CHARGE_PHASE_NONE == cur_charge_phase)
            {
                return;
            }
        }
        else
        {
            // 如果当前在充电
            // adc_sel_ref_voltage(ADC_REF_3_0_VOL); // 3V参考电压
            // adc_sel_pin(ADC_PIN_DETECT_CHARGE);
            // charging_adc_val = adc_getval();
            adc_update_charge_adc_val(ADC_REF_3_0_VOL);

            // 如果充电电压过大，PWM百分比设置为0，等到电压变小才打开
            // if (charging_adc_val >= (u16)((u32)30000 * 4096 / 11 / 3 / 1000)) // 充电电压超过30V
            if (charging_adc_val >= (u16)((u32)28000 * 4096 / 11 / 3 / 1000)) // 充电电压超过 xx V
            {
                pwm_reg = 0;
                TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                TMR1_PWML = pwm_reg & 0xFF;
                return;
            }

            // 如果充电输入电压小于4V，断开充电
            if (charging_adc_val <= (u16)((u32)4000 * 4096 / 11 / 3 / 1000))
            {
                pwm_reg = 0;
                TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                TMR1_PWML = pwm_reg & 0xFF;
                timer1_pwm_disable();

                cur_charge_phase = CUR_CHARGE_PHASE_NONE;
                cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;
                return;
            }

            // 如果已经充满电，直接退出
            if (CUR_CHARGE_PHASE_FULLY_CHARGE == cur_charge_phase)
            {
                return;
            }

            // 可能是对应的MOS管未使能，导致充电电流小，不是涓流充电：
            // if (charging_adc_val <= (u16)((u32)4900 * 4096 / 11 / 3 / 1000)) // 小于4.9V，进行涓流充电
            // {
            //     pwm_reg = (u32)TIMER1_LOW_FEQ_PEROID_VAL * 13 / 100; // 最终的占空比值
            //     TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
            //     TMR1_PWML = pwm_reg & 0xFF;
            //     timer1_set_pwm_low_feq();
            //     cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;
            //     return;
            // }
        }

        // 进入到这里，说明正在充电，且充电电压在 4V~30V之间，不包括4V和30V

        // 检测电池电压，使用内部2.0V参考电压
        // adc_sel_ref_voltage(ADC_REF_2_0_VOL);
        // adc_sel_pin(ADC_PIN_DETECT_BATTERY);
        // bat_adc_val = adc_getval();
        adc_update_bat_adc_val();

        // 刚进入充电，会进入下面这个语句块：
        if (CUR_CHARGE_PHASE_TRICKLE_CHARGE == cur_charge_phase)
        {
            // 如果电池电压小于2.7V，进行涓流充电
            if (bat_adc_val <= (u16)((u32)2700 * 4096 / 2 / 2 / 1000))
            {
                if (CUR_CHARGING_PWM_STATUS_LOW_FEQ != cur_charging_pwm_status)
                {
                    // pwm_reg = (u32)TIMER1_LOW_FEQ_PEROID_VAL * 13 / 100; // 最终的占空比值
                    // TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                    // TMR1_PWML = pwm_reg & 0xFF;
                    timer1_set_pwm_low_feq();
                    cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_LOW_FEQ;
                }

                return;
            }

            // 如果电池电压不小于2.7V，进行正常充电
            cur_charge_phase = CUR_CHARGE_PHASE_NORMAL_CHARGE;
        }

        // 电池电压大于 xx V，从正常充电变为涓流充电
        // if ((bat_adc_val >= (u16)((u32)3400 * 4096 / 2 / 2 / 1000)) &&
        //     (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE != cur_charge_phase))
        // if ((bat_adc_val >= (u16)((u32)(3500 + 100) * 4096 / 2 / 2 / 1000)) && /* 3500 + 100 毫伏，实际测试在 电池电压3.56V，单片机检测脚1.81V都没有进入*/
        //     (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE != cur_charge_phase))
        if ((bat_adc_val >= (u16)((u32)(3500 + 50 + 20) * 4096 / 2 / 2 / 1000)) && /* xxx 毫伏，实际测试在 3.53V，单片机检测脚电压1.793V*/
            (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE != cur_charge_phase))
        {
            trickle_charge_cnt++;
            // pwm_duty >>= 1; // 占空比变为原来的1/2，再看看电池电压会不会变小
            // limited_pwm_duty = pwm_duty;
            // if (limited_pwm_duty > MIN_PWM_DUTY_IN_TRICKLE_CHARGE)
            // {
            //     pwm_duty = limited_pwm_duty - MIN_PWM_DUTY_IN_TRICKLE_CHARGE;
            // }

            if (pwm_duty > MIN_PWM_DUTY_IN_TRICKLE_CHARGE)
            {
                // pwm_duty  -= MIN_PWM_DUTY_IN_TRICKLE_CHARGE; // -=5，调节幅度有点大
                pwm_duty -= 2;
            }

            // 除了检电池电压，还要检占空比，加上这个条件后，约3.55V之后进入涓流充电，进入涓流充电后，测得电池电压3.55V
            if (pwm_duty < 10)
            {
                trickle_charge_cnt++;
            }
#if 0
            pwm_duty = 0;
            pwm_reg = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * 0 / 100; // 寄存器存放的占空比值
            TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
            TMR1_PWML = pwm_reg & 0xFF;
#endif

            /*
                测试发现，
                如果只通过检测电压的方式来累计计数值，
                即使电池从3.55V充到3.57V，很长时间都没有进入下面的条件，
                PWM占空比一直卡在6%~10%
            */
            if (trickle_charge_cnt >= 100)
            {
                trickle_charge_cnt = 0;

                // 准备进入涓流充电，设置PWM，样机最小PWM为4.8%
                pwm_duty = MIN_PWM_DUTY_IN_TRICKLE_CHARGE;
                pwm_reg = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // 最终的占空比值
                TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                TMR1_PWML = pwm_reg & 0xFF;

                cur_charge_phase = CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE;
            }
        }
        else
        {
            trickle_charge_cnt = 0;
        }

        // 如果充电阶段已经到了电池接近满电的阶段
        if (CUR_CHARGE_PHASE_TRICKLE_CHARGE_WHEN_APPROACH_FULLY_CHARGE == cur_charge_phase)
        {
            // static u8 fully_charge_cnt = 0;

            // adc_sel_ref_voltage(ADC_REF_3_0_VOL);
            // adc_sel_pin(ADC_PIN_DETECT_CURRENT);
            // current_adc_val = adc_getval();
            // current = (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 76; //

            // if (bat_adc_val >= (u16)((u32)(3600 + 150) * 4096 / 2 / 2 / 1000))
            // if (bat_adc_val >= (u16)((u32)(3600 + 50) * 4096 / 2 / 2 / 1000)) // 用万用表测试，在3.60~3.61V跳动时，还没有停止充电，等单片机停止充电、PWM输出0%之后，测得电池电压是3.59V，并且电池电压还在下降，最后落在3.44V
            if (bat_adc_val >= (u16)((u32)(3700 + 100) * 4096 / 2 / 2 / 1000)) //
            {

                // fully_charge_cnt++;

                // pwm_duty = MIN_PWM_DUTY_IN_TRICKLE_CHARGE;
                // if (fully_charge_cnt >= 10)
                {
                    // fully_charge_cnt = 0;
                    pwm_reg = 0;
                    TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
                    TMR1_PWML = pwm_reg & 0xFF;
                    timer1_pwm_disable();                             // 已经充满电，断开控制充电的PWM
                    cur_charge_phase = CUR_CHARGE_PHASE_FULLY_CHARGE; // 表示已经充满电，接下来需要等充电电压低于4.0V
                    cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;
                    return;
                }

                // real_bat_adc_val = 0; // 好像可以不加
                // cur_charge_phase = CUR_CHARGE_PHASE_FULLY_CHARGE; // 表示已经充满电，接下来需要等充电电压低于4.0V
                // cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_NONE;
                // fully_charge_cnt = 0;
            }

            return;
        }

        // ===================================================================
        // 以下都是正常充电对应的控制程序，cur_charge_phase == CUR_CHARGE_PHASE_NORMAL_CHARGE
        // ===================================================================

        // 不是正常充电，提前返回
        if (CUR_CHARGE_PHASE_NORMAL_CHARGE != cur_charge_phase)
        {
            return;
        }

        // 如果PWM未切换到高频
        if (CUR_CHARGING_PWM_STATUS_HIGH_FRQ != cur_charging_pwm_status)
        {
            // pwm_reg = MIN_PWM_DUTY_IN_LOW_POWER;
            // TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
            // TMR1_PWML = pwm_reg & 0xFF;
            timer1_set_pwm_high_feq();
            cur_charging_pwm_status = CUR_CHARGING_PWM_STATUS_HIGH_FRQ;
        }

        adc_update_bat_adc_val();
        // 如果电池电压大于 xx V，开始降低功率
        // if (bat_adc_val >= (u16)((u32)3450 * 4096 / 2 / 2 / 1000)) // 3.45V，实际测试是在 3.38V 附近开始降低功率
        // if (bat_adc_val >= (u16)((u32)(3500 + 50) * 4096 / 2 / 2 / 1000)) // 3.55V，实际测试在 3.48V 附近开始降低功率
        // if (bat_adc_val >= (u16)((u32)(3400) * 4096 / 2 / 2 / 1000)) // 在3.36V左右 降低功率（不确定3.36V以下是否就开始降功率，因为电池是从3.3V左右开始充电）
        if (bat_adc_val >= (u16)((u32)(3500 + 50) * 4096 / 2 / 2 / 1000)) //
        {
            // expected_power = 12000;
            // expected_power = 10000; // 功率太高，会直接跳到充满电的阶段（跳到涓流充电，再进入充满电阶段）
            expected_power = (u16)5000; // 充电速度会很慢，1h才提升0.01V~0.02V
        }
        else if (bat_adc_val >= (u16)((u32)(3400) * 4096 / 2 / 2 / 1000)) //
        {
            expected_power = (u16)26500 / 2;
        }
        else // 如果电池电压小于 xx V，按正常的功率进行充电
        {
            expected_power = (u16)26500;
        }

        // adc_sel_ref_voltage(ADC_REF_3_0_VOL);
        // adc_sel_pin(ADC_PIN_DETECT_CURRENT);
        // current_adc_val = adc_getval();
        adc_update_current_adc_val();

        /*
            检测电流引脚检测到的电压 == ad值 / 4096 * 参考电压
            current_adc_val * 3 / 4096

            检测电流的引脚检测到的充电电流 == 检测电流引脚检测到的电压 / 110(运放放大倍数) / 0.005R，
            逐步换成单片机可以计算的形式：
            current_adc_val * 3 / 4096 / 110 / 0.005
            current_adc_val * 3 / 4096 / 110 * 1000 / 5
            current_adc_val * 3 * 1000 / 5 / 4096 / 110
            得到的是以A为单位的电流，需要再转换成以mA为单位：
            current_adc_val * 3 * 1000 * 1000 / 5 / 4096 / 110，计算会溢出，需要再化简
            (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110
            current =  (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 110;
        */
        // current = (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 76; // 计算电流，单位：mA
        current = (u32)current_adc_val * 3 * 1000 * (1000 / 5) / 4096 / 65; // 计算电流，单位：mA

        /*
            计算充电电压
        */
        // voltage_of_charging = (u32)charging_adc_val * 3 * 1000 * 11 / 4096;
        /*
            计算电池电压
            电池电压（mV） == 采集到的ad值 / 4096 * 参考电压 * 分压系数 * 1000（mV）
        */
        voltage_of_bat = (u32)bat_adc_val * 2 * 1000 * 2 / 4096; // 计算电池电压，单位：mV

#if 0

        // if (bat_adc_val >= (u16)((u32)3500 * 4096 / 2 / 2 / 1000))
        // if (bat_adc_val >= (u16)((u32)3500 * 4096 / 2 / 2 / 1000)) // 实际在电池3.45V开始就进入这里
        if (bat_adc_val >= (u16)((u32)3550 * 4096 / 2 / 2 / 1000)) // 实际测试在电池3.50V左右开始进入
        {
            // 开始涓流充电
            // limited_current = 100;
            // if (current > 100)
            // if (current > 200) // 超过3.55V之后，好像会充不进电
            if (current > 400) //
            // if (current > 800) //
            {
                if (pwm_duty > 0)
                {
                    pwm_duty--;
                    // pwm_duty = 0; // 刚从正常充电变为涓流充电，如果pwm占空比没有迅速调节，电池电压有可能会直接跳到3.6V，认为充满电
                }
            }
            // else if (current < 100)
            // else if (current < 200) // 超过3.55V之后，好像会充不进电
            else if (current < 400)
            // else if (current < 800)
            {
                if (pwm_duty < 100) // 100%占空比
                {
                    pwm_duty++;
                }
            }
        }
        else // 如果电池电压不超过 xx V
#endif
        {

            // 如果检测到电流的ad值已经爆表
            if (current_adc_val >= 4095)
            // if (current >= 5400) // 如果电流值已经爆表，超过单片机能检测的值（理论值）：5454.54
            {
                // printf("current overflow\n");
                if (pwm_duty > MIN_PWM_DUTY_IN_LOW_POWER)
                {
                    pwm_duty--;
                }
            }
            else //
            {
                // power = voltage_of_charging * current / 1000; // 1000mV * 1000mA == 1000000 (1 Watt)
                // 1000mV * 1000mA == 1000000 (1 Watt)
                if (expected_power != 26500)
                {
                    // 0.728V电压，对应的电流是 1915 mA
                    if (current < 1916)
                    {
                        current = 1;
                    }
                }

                power = (u32)voltage_of_bat * current / 1000; // 计算功率，单位：毫瓦

                if (power < expected_power) // 如果当前的功率 小于 限制的功率
                {
                    if (pwm_duty < 100) // 100%占空比
                    {
                        pwm_duty++;
                    }
                }
                else if (power > expected_power) // 如果当前的功率 大于 限制的功率
                {
                    if (pwm_duty > MIN_PWM_DUTY_IN_LOW_POWER) // 防止向下溢出
                    {
                        pwm_duty--;
                    }
                }
                else // power == 目标值，不做调节
                {
                }

                /*
                    如果已经降低功率，显示控制充电的pwm占空比

                    接近满电时，测得的电流值已经不准确，即使单片机控制充电的pwm不输出，测电流的引脚上还有0.728V电压
                */
                // if (expected_power < 26500)
                // {
                //     // 限制为60%，感觉没有起到效果
                //     // 限制为40%，发现限制太死，跟涓流充电没有区别
                //     /*
                //         限制为50%，如果输入电压大于5V，才有限流的效果，
                //         如果在5V附近，充电电流会在 0.8A ~ 0.0xA 来回跳动，
                //         此时pwm占空比变化应该是±1%，应该是mos管导通和不导通造成的
                //     */
                //     if (pwm_duty > (u8)60)
                //     {
                //         pwm_duty = (u8)60;
                //     }
                // }
            }
        }

        // printf("pwm_duty : %bu %%\n", pwm_duty);
        pwm_reg = (u32)TIMER1_HIGH_FEQ_PEROID_VAL * pwm_duty / 100; // 最终的占空比值

        //     // printf("pwm_duty :%u\n", pwm_duty);
        TMR1_PWMH = (pwm_reg >> 8) & 0xFF;
        TMR1_PWML = pwm_reg & 0xFF;
    }
}
