#include "uart0.h"

/*********************************
|--------------------------------|
| 使用%d打印的数值不对时，请注意 |
|--------------------------------|
| 格式 |     含义     | 针对类型 |
|------|--------------|----------|
| %bd  | 单个字节变量 | char     |
| % d  | 两个字节变量 | int      |
| %ld  | 四个字节变量 | long int |
|--------------------------------|
**********************************/

// 重写putchar()函数
char putchar(char uart_data)
{
    UART0_DATA = uart_data;
    while (!(UART0_STA & UART_TX_DONE(0x1)))
        ; // 等待发送完成

    return uart_data;
}

void uart0_config(void)
{
    // 初始化打印
    // TX
 

    P2_MD1 &= ~GPIO_P27_MODE_SEL(0x03);
    P2_MD1 |= GPIO_P27_MODE_SEL(0x01); // 输出模式
    FOUT_S27 = GPIO_FOUT_UART0_TX;     // 复用为串口输出
 
    UART0_BAUD1 = (((SYSCLK - UART0_BAUD) / UART0_BAUD) >> 8) & 0xFF;
    UART0_BAUD0 = ((SYSCLK - UART0_BAUD) / UART0_BAUD) & 0xFF;
    UART0_CON0 = UART_STOP_BIT(0x0) |
                 UART_EN(0x1); // 8bit数据，1bit停止位
}