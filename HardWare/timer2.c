#include "timer2.h"

// pwm配置
void timer2_pwm_config(void)
{
    // 配置 xx 为 PWM 输出端口
    // P2_MD0 &= ~GPIO_P22_MODE_SEL(0x3);
    // P2_MD0 |= GPIO_P22_MODE_SEL(0x1); // 输出模式
    // FOUT_S22 = GPIO_FOUT_TMR2_PWMOUT; 

    P3_MD0 &= ~GPIO_P30_MODE_SEL(0x3);
    P3_MD0 |= GPIO_P30_MODE_SEL(0x1); // 输出模式
    FOUT_S30 = GPIO_FOUT_TMR2_PWMOUT;

    TMR_ALLCON = TMR2_CNT_CLR(0x1);                        // 清除计数值
    TMR2_PRH = TMR_PERIOD_VAL_H((TIMER2_FEQ >> 8) & 0xFF); // 周期值
    TMR2_PRL = TMR_PERIOD_VAL_L((TIMER2_FEQ >> 0) & 0xFF);
    TMR2_PWMH = TMR_PWM_VAL_H((((u32)TIMER2_FEQ * 50 / 100) >> 8) & 0xFF); // 占空比设置值
    TMR2_PWML = TMR_PWM_VAL_L((((u32)TIMER2_FEQ * 50 / 100) >> 0) & 0xFF);
    TMR2_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                           // 使能计数中断
    TMR2_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x07) | TMR_MODE_SEL(0x2); // 选择系统时钟，时钟源 128 分频，PWM模式
}

// 开启PWM，引脚复用为pwm输出
void timer2_pwm_enable(void)
{
    // FOUT_S22 = GPIO_FOUT_TMR2_PWMOUT;

    FOUT_S30 = GPIO_FOUT_TMR2_PWMOUT;

    TMR2_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x07) | TMR_MODE_SEL(0x2); // 选择系统时钟，时钟源 128 分频，PWM模式
}

// 关闭pwm，引脚输出低电平
void timer2_pwm_disable(void)
{
    TMR2_CONL &= ~(0x03 << 0);    // 不使能定时器计数

    // FOUT_S22 = GPIO_FOUT_AF_FUNC; // 数字复用功能输出
    // P22 = 0;

    FOUT_S30 = GPIO_FOUT_AF_FUNC; // 数字复用功能输出
    P30 = 0;
}


// pwm_duty_val占空比值，不是以百分比为单位，直接将该参数写入寄存器
void timer2_set_pwm_duty(u16 pwm_duty_val)
{
    // TMR2_PWMH = TMR_PWM_VAL_H((((u32)TIMER2_FEQ * pwm_duty / 100) >> 8) & 0xFF); // 占空比设置值
    // TMR2_PWML = TMR_PWM_VAL_L((((u32)TIMER2_FEQ * pwm_duty / 100) >> 0) & 0xFF);

    // TMR2_PWMH = TMR_PWM_VAL_H((((u32)TIMER2_FEQ * pwm_duty_val / 100) >> 8) & 0xFF); // 占空比设置值
    // TMR2_PWML = TMR_PWM_VAL_L((((u32)TIMER2_FEQ * pwm_duty_val / 100) >> 0) & 0xFF);

    TMR2_PWMH = (pwm_duty_val >> 8) & 0xFF; // 占空比设置值
    TMR2_PWML = (pwm_duty_val >> 0) & 0xFF;
}


