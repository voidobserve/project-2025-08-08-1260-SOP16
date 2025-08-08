#include "timer1.h"

// pwm配置，默认为低频，固定占空比
void timer1_pwm_config(void)
{
    //  配置 P26 为 PWM 输出端口
    P2_MD1 &= ~GPIO_P26_MODE_SEL(0x3);
    P2_MD1 |= GPIO_P26_MODE_SEL(0x1); // 输出模式
    FOUT_S26 = GPIO_FOUT_TMR1_PWMOUT;

    TMR_ALLCON = TMR1_CNT_CLR(0x1);                                       // 清除计数值
    TMR1_PRH = TMR_PERIOD_VAL_H((TIMER1_LOW_FEQ_PEROID_VAL >> 8) & 0xFF); // 周期值
    TMR1_PRL = TMR_PERIOD_VAL_L((TIMER1_LOW_FEQ_PEROID_VAL >> 0) & 0xFF);
    TMR1_PWMH = TMR_PWM_VAL_H((((u32)TIMER1_LOW_FEQ_PEROID_VAL * 0 / 100) >> 8) & 0xFF); // 占空比设置值
    TMR1_PWML = TMR_PWM_VAL_L((((u32)TIMER1_LOW_FEQ_PEROID_VAL * 0 / 100) >> 0) & 0xFF);
    TMR1_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);  // 使能计数中断
    TMR1_CONL = TMR_SOURCE_SEL(0x7) | TMR_MODE_SEL(0x2); // 选择系统时钟，时钟源不分频，PWM模式
}

// 将pwm设置为高频率、0%占空比
void timer1_set_pwm_high_feq(void)
{
    FOUT_S26 = GPIO_FOUT_TMR1_PWMOUT;

    // 104KHz
    TMR_ALLCON = TMR1_CNT_CLR(0x1);                                        // 清除计数值
    TMR1_PRH = TMR_PERIOD_VAL_H((TIMER1_HIGH_FEQ_PEROID_VAL >> 8) & 0xFF); // 周期值
    TMR1_PRL = TMR_PERIOD_VAL_L((TIMER1_HIGH_FEQ_PEROID_VAL >> 0) & 0xFF);
    TMR1_PWMH = TMR_PWM_VAL_H((((u32)TIMER1_HIGH_FEQ_PEROID_VAL * 0 / 100) >> 8) & 0xFF); // 占空比设置值
    TMR1_PWML = TMR_PWM_VAL_L((((u32)TIMER1_HIGH_FEQ_PEROID_VAL * 0 / 100) >> 0) & 0xFF);
    TMR1_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);  // 使能计数中断
    TMR1_CONL = TMR_SOURCE_SEL(0x7) | TMR_MODE_SEL(0x2); // 选择系统时钟，不分频，PWM模式
}

// 将pwm设置为低频率，固定占空比
void timer1_set_pwm_low_feq(void)
{
    FOUT_S26 = GPIO_FOUT_TMR1_PWMOUT;

    TMR_ALLCON = TMR1_CNT_CLR(0x1);                                       // 清除计数值
    TMR1_PRH = TMR_PERIOD_VAL_H((TIMER1_LOW_FEQ_PEROID_VAL >> 8) & 0xFF); // 周期值
    TMR1_PRL = TMR_PERIOD_VAL_L((TIMER1_LOW_FEQ_PEROID_VAL >> 0) & 0xFF);
    TMR1_PWMH = TMR_PWM_VAL_H((((u32)TIMER1_LOW_FEQ_PEROID_VAL * 18 / 100) >> 8) & 0xFF); // 占空比设置值
    TMR1_PWML = TMR_PWM_VAL_L((((u32)TIMER1_LOW_FEQ_PEROID_VAL * 18 / 100) >> 0) & 0xFF);
    TMR1_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);  // 使能计数中断
    TMR1_CONL = TMR_SOURCE_SEL(0x7) | TMR_MODE_SEL(0x2); // 选择系统时钟，时钟源1分频，PWM模式
}

void timer1_set_pwm_duty(u8 duty)
{
    TMR1_PWMH = (duty >> 8) & 0xFF;
    TMR1_PWML = duty & 0xFF;
}

void timer1_pwm_enable(void)
{
    FOUT_S26 = GPIO_FOUT_TMR1_PWMOUT;
    TMR1_CONL |= (0x02 << 0); // PWM模式
}

// 关闭pwm，引脚输出低电平
void timer1_pwm_disable(void)
{
    TMR1_CONL &= ~(0x03 << 0);    // 不使能定时器计数
    FOUT_S26 = GPIO_FOUT_AF_FUNC; // 数字复用功能输出
    P26 = 0;
}
