#ifndef UART_H
#define UART_H

#include "hal_data.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

extern volatile uint8_t g_rx_char;
extern volatile bool    g_rx_flag;
extern volatile int32_t k210_x;
extern volatile int32_t k210_y;
extern volatile bool    k210_ready;

void UART_Init(void);
void UART_Printf(const char * format, ...);

#endif
