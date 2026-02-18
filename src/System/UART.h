#ifndef UART_H
#define UART_H

#include "hal_data.h"
#include <stdio.h>
#include <stdarg.h>

/* 全局变量声明 (供外部查询状态) */
extern volatile bool uart_send_complete_flag;
extern volatile uint8_t g_rx_char;
extern volatile bool    g_rx_flag;

/* 函数声明 */
void UART_Init(void);
void UART_Printf(const char * format, ...);

#endif
