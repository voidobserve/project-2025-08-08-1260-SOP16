#include "low_energy.h"

void sleep_in(void)
{
    SYS_CON6 |= SYS_MPDN_CNT(0x3);     // 关闭程序存储器供电的延迟时间配置4个系统周期
    SYS_CON7 = (SYS_EXT_SLP_CNT(0x3) | // 退出低功耗延迟的时间配置各配四个系统周期
                SYS_MTPUP_CNT(0x3) |
                SYS_OPM_LDO_CNT(0x3) |
                SYS_CLSM_LDO_CNT(0x3));
    SYS_CON8 |= SYS_LPSLP_DIS_ANA(0x1);   // 打开低功耗sleep mode一键关模拟模块功能(即关闭TK,AMP,CMP,ADC之类的模块)
    CLK_XOSC &= ~(CLK_XOSC_LOW_EN(0x1) |  // 关闭32.768KHz低速晶振
                  CLK_XOSC_HIGH_EN(0x1)); // 关闭高速晶振
    LVD_CON0 &= ~(LVD_VCC_DETE_EN(0x1) |  // 关闭VCC电源VCC电压低电检测功能
                  LVD_VDD_DETE_EN(0x1));  // 关闭1.5V数字逻辑系统工作电压VDD低电检测功能
    PMU_CON2 &= ~0x70;                    // 关闭VPTAT_ADC输出，关闭温度传感器输出VPTAT，主LDO过流档位选择50mA
    CLK_CON0 &= ~CLK_SYSCLK_SEL(0x3);     // 系统时钟选择rc64k
    FLASH_TIMEREG1 = 0x0;                 // 配置LIRC时FLASH访问速度
    PMU_CON0 &= ~0x60;                    // 关闭VDD POR模块, 关闭VBG06_REF输出
    CLK_ACON0 &= ~CLK_AIP_HRC_EN(0x1);    // 关闭HRC时钟
    LP_CON = (LP_IDLE_EN(0x1) |
              LP_SLEEP_GO_EN(0x1) | // Sleep低功耗模式唤醒后继续跑后续程序
              LP_GLIRC_EN(0x1) |    // 关闭RC64K低速时钟
              LP_SLEEP_EN(0x1));    // 使能睡眠
}

void sleep_out(void)
{
    SYS_CON8 &= ~SYS_LPSLP_DIS_ANA(0x1); // 关闭低功耗sleep mode一键关模拟模块功能(即打开TK,AMP,CMP,ADC之类的模块)
    PMU_CON0 |= 0x60;                    // 使能VDD POR模块，使能VBG06_REF输出
    LVD_CON0 |= (LVD_VCC_DETE_EN(0x1) |  // 使能VCC电源VCC电压低电检测功能
                 LVD_VDD_DETE_EN(0x1));  // 使能1.5V数字逻辑系统工作电压VDD低电检测功能
    PMU_CON2 |= 0x70;                    // 使能VPTAT_ADC输出，使能温度传感器输出VPTAT，主LDO过流档位选择100mA
    CLK_ACON0 |= CLK_AIP_HRC_EN(0x1);    // 使能HRC时钟
    // LP_WKPND |= LP_WKUP_0_PCLR(0x1);     // 清除通道0唤醒标志位

    LP_WKPND |= LP_WKUP_0_PCLR(0x01) | LP_WKUP_1_PCLR(0x01); // 清除唤醒通道0唤醒标志位，清除唤醒通道1唤醒标志位

    FLASH_TIMEREG1 = 0x58;           // FLASH访问速度 = 系统时钟/3
    CLK_CON0 |= CLK_SYSCLK_SEL(0x3); // 系统时钟选择hirc_clk
}

// 低功耗
void low_energy(void)
{
    // #define WUT_PERIOD_VAL (SYSCLK / 64 / 3000 - 1)

    bit flag_is_really_wake_up = 0; // 标志位，是否真的要唤醒

    if (CUR_LED_MODE_OFF != cur_led_mode || /* 指示灯没有关闭 */
        0 != cur_light_pwm_duty_val)        /* 主灯光的占空比不为0 */
    {
        // 不满足进入低功耗的条件，退出
        return;
    }

    // printf("begin sleep\n");

    // 低功耗模式下配置所有的IO端口为模拟输入模式
    P0_MD0 = 0xFF;
    P0_MD1 = 0xFF;
    P1_MD0 = 0xFF;
    P1_MD1 = 0xFF;
    P2_MD0 = 0xFF;
    P2_MD1 = 0xFF;
    P3_MD0 = 0xFF;
    P3_MD1 = 0xFF;

// 关闭不使用的片上外设
#if USE_MY_DEBUG
    UART0_CON0 &= ~UART_EN(0x01); // 不使能 uart0 
#endif
    // __DisableIRQ(TMR0_IRQn); // 关闭中断
    // TMR0_CONL &= ~( (0x07 << 5) | (0x03 << 0)); // 清空计数源配置，清空计数模式配置--不使能计数
    // TMR0_CONL |= (0x05 << 5); // 不选择计数源--无计数源
    TMR0_CONL &= ~(0x03 << 0); // 清空计数模式配置--不使能计数
    TMR1_CONL &= ~(0x03 << 0); // 清空计数模式配置--不使能计数
    TMR2_CONL &= ~(0x03 << 0); // 清空计数模式配置--不使能计数

    ADC_CFG0 &= ~((0x01 << 6) | (0x01 << 3)); // 不使能 adc，不使能通道0转换功能
    AMP_CON11 &= ~(0x01 << 0);                // 不使能 pga0

    // 控制充电的引脚，输出低电平
    // P2_MD1 &= ~GPIO_P26_MODE_SEL(0x03);
    // P2_MD1 |= GPIO_P26_MODE_SEL(0x01); // 输出模式
    // P26 = 0;

    // 控制主灯光的引脚，输出电平

    // P17是烧录引脚，没调用IO_MAP关闭烧录功能情况下需要将此引脚上拉
    // P1_PU |= GPIO_P17_PULL_UP(0x1);

    while (0 == flag_is_really_wake_up) // 如果不是真的被唤醒，则回到低功耗
    {

        // printf("sleep\n");
        /*
            配置唤醒源，
            红外接收，有低电平就唤醒，连接到唤醒通道0
            充电电压 大于 4.9V 唤醒

            使用定时唤醒，唤醒后检测一次充电电压，如果小于4.9V，回到低功耗
        */
        // 红外接收引脚：
        P2_PU |= GPIO_P23_PULL_UP(0x01);      // 上拉
        P2_MD0 &= ~(GPIO_P23_MODE_SEL(0x03)); // 输入模式
        FIN_S10 = 0x14;                       // 唤醒通道0选择 红外接收引脚

        // 充电电压检测引脚配置为输入模式，关闭上下拉
        P2_PU &= ~GPIO_P25_PULL_UP(0x01);   // 关闭上拉
        P2_PD &= ~GPIO_P25_PULL_PD(0x01);   // 关闭下拉
        P2_MD1 &= ~GPIO_P25_MODE_SEL(0x03); // 输入模式

        FIN_S11 = 0x16; // 唤醒通道1 选择 P25 作为唤醒源

#if 0
    // 配置定时唤醒：
    // 设置WUT的计数功能，配置一个3000ms的中断
    // __EnableIRQ(WUT_IRQn);         // 使能WUT中断
    IE_EA = 1;                     // 使能总中断
    // TMR_ALLCON = WUT_CNT_CLR(0x1); // 清除计数值
// WUT_PRH = TMR_PERIOD_VAL_H(((3000 - 1) >> 8) & 0xFF); // 周期值
// WUT_PRL = TMR_PERIOD_VAL_L(((3000 - 1) >> 0) & 0xFF);
    WUT_PRH = TMR_PERIOD_VAL_H((WUT_PERIOD_VAL >> 8) & 0xFF); // 周期值
    WUT_PRL = TMR_PERIOD_VAL_L((WUT_PERIOD_VAL >> 0) & 0xFF);
    WUT_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                          // 使能唤醒定时器
    WUT_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x6) | TMR_MODE_SEL(0x1); // 选择系统时钟，64分频，计数模式
#endif

        LP_CON &= ~LP_ISD_DIS_LP_EN(0x01); // 使能ISD模式下低功耗功能
        LP_WKPND = LP_WKUP_0_PCLR(0x01) |  /* 清除唤醒标志位 */
                   LP_WKUP_1_PCLR(0x1);    /* 清除唤醒标志位 */
        // LP_WKCON = (LP_WKUP_0_EDG(0x01) |  /* 通道0低电平触发唤醒 */
        //             LP_WKUP_0_EN(0x01) |   /* 唤醒通道0使能 */
        //             LP_WKUP_2_EDG(0x00) |  /* 通道2高电平触发唤醒 */
        //             LP_WKUP_2_EN(0x01));   /* 唤醒通道2使能（WUT唤醒只能是唤醒通道2） */

        LP_WKCON = LP_WKUP_0_EDG(0x01) | /* 通道0低电平触发唤醒 */
                   LP_WKUP_0_EN(0x01) |  /* 唤醒通道0使能 */
                   LP_WKUP_1_EDG(0x00) | /* 通道1高电平触发唤醒 */
                   LP_WKUP_1_EN(0x01);   /* 唤醒通道1使能 */

        sleep_in();
        sleep_out();

        /* 唤醒后，关闭唤醒源 */
        LP_WKCON &= ~(LP_WKUP_0_EN(0x01) | LP_WKUP_1_EN(0x01)); // 不使能唤醒通道0，不使能唤醒通道1

        // 如果不是红外信号唤醒 或是 充电电压小于4.9V，重新回到低功耗
        if (0 == P23)
        {
            flag_is_really_wake_up = 1;

            // uart0_config();
            // printf("ir wake up\n");
            continue; // 提前退出，防止红外解码失败
        }

        adc_config();
        adc_update_charge_adc_val(ADC_REF_2_0_VOL);                      // adc使用2V参考电压，更新 charging_adc_val 的数值
        if (charging_adc_val >= (u16)((u32)4900 * 4096 / 11 / 2 / 1000)) // 如果此时充电电压小于4.9V
        {
            flag_is_really_wake_up = 1;

            // uart0_config();
            // printf("charge wake up\n");
            continue;
        }
    }

    // printf("wake up\n");

    // 唤醒后，需要重新初始化系统
    user_init();

    // printf("wake up\n");
}

// 使用了WUT中断唤醒，必须要加上对应的中断服务函数
// void WUT_IRQHandler(void) interrupt WUT_IRQn
// {
//     // 进入中断设置IP，不可删除
//     __IRQnIPnPush(WUT_IRQn);

//     // ---------------- 用户函数处理 -------------------

//     // 周期中断
//     if(WUT_CONH & TMR_PRD_PND(0x1)) {
//         WUT_CONH |=  TMR_PRD_PND(0x1);          // 清除pending

//     }

//     // 退出中断设置IP，不可删除
//     __IRQnIPnPop(WUT_IRQn);
// }
