#include "adc.h"

volatile u16 adc_val = 0;

void adc_config(void)
{
    // 检测充电电压的引脚
    P2_MD1 |= GPIO_P25_MODE_SEL(0x03); // 模拟模式
    // 检测电池电压的引脚
    P2_MD1 |= GPIO_P24_MODE_SEL(0x03); // 模拟模式
    // 检测充电电流（流入电池的电流）的引脚
    P2_MD0 |= GPIO_P21_MODE_SEL(0x03); // 模拟模式

    ADC_CFG1 |= (0x0F << 3); // ADC时钟分频为16分频，为系统时钟/16
    ADC_CFG2 = 0xFF;         // 通道0采样时间配置为256个采样时钟周期

    ADC_ACON1 &= ~((0x01 << 5) |   /* 关闭ADC外部参考选择信号 */
                   (0x07 << 0));   /* 清空ADC内部参考电压的选择配置 */
    ADC_ACON1 |= (0x01 << 6) |     /* 使能adc内部参考 */
                 (0x03 << 3) |     /* 关闭测试信号 */
                 (0x01 << 0);      /* 选择 内部 2.0V 作为参考电压 */
    ADC_ACON0 = ADC_CMP_EN(0x1) |  // 打开ADC中的CMP使能信号
                ADC_BIAS_EN(0x1) | // 打开ADC偏置电流能使信号
                ADC_BIAS_SEL(0x1); // 偏置电流选择：1x
    ADC_TRGS0 |= (0x07 << 4);      // 通道 0DLY 的 ADC 时钟个数选择，配置为 4n+1，4 * 29 + 1
    ADC_CHS0 |= (0x01 << 6);       // 使能 通道 0DLY 功能
}

/**
 * @brief 切换adc的参考电压
 *
 * @param adc_ref_voltage
 *              ADC_REF_2_0_VOL = 0x00, // adc使用2.0V参考电压
                ADC_REF_3_0_VOL,        // adc使用3.0V参考电压
 */
void adc_sel_ref_voltage(u8 adc_ref_voltage)
{
    ADC_ACON1 &= ~((0x01 << 5) | /* 关闭ADC外部参考选择信号 */
                   (0x07 << 0)); /* 清空ADC内部参考电压的选择配置 */

    if (ADC_REF_2_0_VOL == adc_ref_voltage)
    {
        ADC_ACON1 |= (0x01 << 6) | /* 使能adc内部参考 */
                     (0x03 << 3) | /* 关闭测试信号 */
                     (0x01 << 0);  /* 选择 内部 2.0V 作为参考电压 */
    }
    else if (ADC_REF_3_0_VOL == adc_ref_voltage)
    {
        ADC_ACON1 |= (0x01 << 6) | /* 使能adc内部参考 */
                     (0x03 << 3) | /* 关闭测试信号 */
                     (0x03 << 0);  /* 选择 内部 3.0V 作为参考电压 */
    }

    ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能通道0转换
                ADC_EN(0x1);        // 使能A/D转换
    delay_ms(1);                    // 等待adc稳定
}

/**
 * @brief 切换检测ad的引脚（函数内部只切换引脚，不切换参考电压）
 *
 * @param adc_pin
 * @return * void
 */
void adc_sel_pin(u8 adc_pin)
{
    // ADC_CFG0 &= ~((0x01 << 6) | (0x01 << 3)); // 关闭adc，不使能通道0转换
    ADC_CHS0 &= ~((0x01 << 7) | /* 数据格式左对齐 */
                  (0x01 << 5) | // 选择内部通道
                  (0x01 << 4) |
                  (0x01 << 3) |
                  (0x01 << 2) |
                  (0x01 << 1) |
                  (0x01 << 0)); // 清空选择的adc0通路

    if (ADC_PIN_DETECT_CHARGE == adc_pin)
    {
        // 检测充电电压的引脚
        ADC_CHS0 |= (0x15 << 0); // P25 对应的模拟通道
    }
    else if (ADC_PIN_DETECT_BATTERY == adc_pin)
    {
        // 检测电池电压的引脚
        ADC_CHS0 |= (0x14 << 0); // P24 对应的模拟通道
    }
    else if (ADC_PIN_DETECT_CURRENT == adc_pin)
    {
        // 检测电流的引脚
        ADC_CHS0 |= (0x11 << 0); // P21 对应的模拟通道
    }

    ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能通道0转换
                ADC_EN(0x1);        // 使能A/D转换
    delay_ms(1);                    // 等待adc稳定
}

// adc采集+滤波
u16 adc_getval(void)
{
    u8 i; // adc采集次数的计数（为了节省程序空间，这里没有给初始值，由下面的语句给初始值）
    volatile u16 g_temp_value = 0;
    volatile u32 g_tmpbuff = 0;
    volatile u16 g_adcmax = 0;
    volatile u16 g_adcmin = 0xFFFF;

    // 采集20次，去掉前两次采样，再去掉一个最大值和一个最小值，再取平均值
    for (i = 0; i < 20; i++)
    {
        ADC_CFG0 |= ADC_CHAN0_TRG(0x1); // 触发ADC0转换
        while (!(ADC_STA & ADC_CHAN0_DONE(0x1)))
            ;                                                 // 等待转换完成
        g_temp_value = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4); // 读取ADC0的值
        ADC_STA = ADC_CHAN0_DONE(0x1);                        // 清除ADC0转换完成标志位

        if (i < 2)
            continue; // 丢弃前两次采样的
        if (g_temp_value > g_adcmax)
            g_adcmax = g_temp_value; // 最大
        if (g_temp_value < g_adcmin)
            g_adcmin = g_temp_value; // 最小

        g_tmpbuff += g_temp_value;
    }

    g_tmpbuff -= g_adcmax;           // 去掉一个最大
    g_tmpbuff -= g_adcmin;           // 去掉一个最小
    g_temp_value = (g_tmpbuff >> 4); // 除以16，取平均值

    return g_temp_value;
}

/**
 * @brief 更新电池对应的ad值，内部使用2.0V参考电压
 *
 */
void adc_update_bat_adc_val(void)
{
    adc_sel_ref_voltage(ADC_REF_2_0_VOL);
    adc_sel_pin(ADC_PIN_DETECT_BATTERY);
    bat_adc_val = adc_getval();
}

/**
 * @brief 更新充电对应的ad值
 *
 * @param adc_ref_voltage
 *          ADC_REF_2_0_VOL 使用2.0V作为参考电压
 *          ADC_REF_3_0_VOL 使用3.0V作为参考电压
 */
void adc_update_charge_adc_val(u8 adc_ref_voltage)
{
    adc_sel_ref_voltage(adc_ref_voltage);
    adc_sel_pin(ADC_PIN_DETECT_CHARGE);
    charging_adc_val = adc_getval();
}

/**
 * @brief 更新电流对应的ad值，内部使用3.0V参考电压
 *
 */
void adc_update_current_adc_val(void)
{
    adc_sel_ref_voltage(ADC_REF_3_0_VOL);
    adc_sel_pin(ADC_PIN_DETECT_CURRENT);
    current_adc_val = adc_getval();
}

#if 0  // 滑动平均
/* 滑动平均 */
static volatile u16 bat_adc_val_samples[BAT_ADC_VAL_SAMPLE_COUNT];
static volatile u8 bat_adc_val_sample_index = 0;
u16 get_filtered_bat_adc_val(u16 bat_adc_val)
{
    u8 i = 0;
    u32 sum = 0;
    bat_adc_val_samples[bat_adc_val_sample_index++] = bat_adc_val;
    if (bat_adc_val_sample_index >= BAT_ADC_VAL_SAMPLE_COUNT)
        bat_adc_val_sample_index = 0;

    for (i = 0; i < BAT_ADC_VAL_SAMPLE_COUNT; i++)
        sum += bat_adc_val_samples[i];

    return sum / BAT_ADC_VAL_SAMPLE_COUNT;
}
#endif // 滑动平均