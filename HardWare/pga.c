#include "pga.h"

/*
    SOP16封装只能使用
    P26、P27、P30
    P21、P22、P23
    这两组作为运放的引脚
*/

void pga_config(void)
{
    // Note：如果同时使用AMP0和AMP2，则需要配置AMP_CON8[0] = 0;AMP_CON8[1] = 0，关闭使能，如以下两行代码
    AMP_CON8 &= ~(PGA0_IP_TO_PGA2_IN_EN(0x1) | // 关闭PGA2的IN选通到PGA0的IP使能
                  PGA0_IN_TO_PGA2_IP_EN(0x1)); // 关闭PGA2的IP选通到PGA0的IN使能

    // 配置AMP0的IO    P21--OP0_P  P22--OP0_N  P23--OP0_O
    P2_MD0 |= GPIO_P21_MODE_SEL(0x3) | // P21\P22\P23设为模拟模式
              GPIO_P22_MODE_SEL(0x3) |
              GPIO_P23_MODE_SEL(0x3);

    AMP_CON7 |= AMP0_FB_SEL(0x1);      // 使能AMP0内部反馈通路
    AMP_CON8 |= AMP0_IN2_IO(0x1);      // 使能AMP0负向输入端到IO的通路
    AMP_CON9 |= AMP0_IP2_IO_EN(0x1);   // 使能AMP0正向输入端到IO的通路
    AMP_CON10 |= AMP0_OUT2_IO_EN(0x1); // 使能AMP0输出端到IO的通路
    AMP_CON1 = AMP_PGA0_RIN(0x06) |    // AMP0内置负向输入电阻选择 2.5 KΩ
               AMP_PGA0_RFB(0x1) |     // AMP0内置反馈电阻选择 160 KΩ     (放大倍数= (RFB+RIN)/RIN == 65 )
               AMP_PGA0_IB(0x3);       // 总偏置电流和输出电流选择大电流
    AMP_CON11 |= AMP_PGA0_EN(0x1);     // 使能AMP0
}
