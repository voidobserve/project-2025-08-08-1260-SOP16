#ifndef __UART0_H
#define __UART0_H

#include "include.h"

#define UART0_BAUD (115200UL)
#define USER_UART0_BAUD ((SYSCLK - UART0_BAUD) / (UART0_BAUD))

void uart0_config(void);

#endif