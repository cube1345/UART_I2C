#include "UART.h"
#include "TrackingPID.h"
#include <stdlib.h> // 必须包含，修复 atoi 警告
#include <string.h>

volatile uint8_t g_rx_char = 0;
volatile bool    g_rx_flag = false;
volatile int32_t k210_x = 0;
volatile int32_t k210_y = 0;
volatile bool    k210_ready = false;
volatile bool    uart9_tx_complete = false;

void UART_Init(void) {
    R_SCI_UART_Open(&g_uart9_ctrl, &g_uart9_cfg);
    R_SCI_UART_Open(&g_uart4_ctrl, &g_uart4_cfg);
    R_SCI_UART_Open(&g_uart_k210_ctrl, &g_uart_k210_cfg);
}

void uart9_callback(uart_callback_args_t *p_args) {
    if (p_args->event == UART_EVENT_RX_CHAR) {
        g_rx_char = (uint8_t)p_args->data;
        g_rx_flag = true;
    }
    if (p_args->event == UART_EVENT_TX_COMPLETE) uart9_tx_complete = true;
}

void uart4_callback(uart_callback_args_t *p_args) {
    if (p_args->event == UART_EVENT_RX_CHAR) {
        g_rx_char = (uint8_t)p_args->data;
        g_rx_flag = true;
    }
}

void k210_uart_callback(uart_callback_args_t *p_args) {
    if (p_args->event == UART_EVENT_RX_CHAR) {
        // 直接在此处通过指针修改全局变量
        if (K210_Protocol_Parse((uint8_t)p_args->data, (int32_t *)&k210_x, (int32_t *)&k210_y)) {
            k210_ready = true;
        }
    }
}

void UART_Printf(const char * format, ...) {
    char buff[128];
    va_list args;
    va_start(args, format);
    uint32_t len = (uint32_t)vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);
    uart9_tx_complete = false;
    R_SCI_UART_Write(&g_uart9_ctrl, (uint8_t *)buff, len);
    uint32_t timeout = 0xFFFF;
    while(!uart9_tx_complete && timeout-- > 0);
}
