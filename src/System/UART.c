#include "UART.h"

volatile bool uart_send_complete_flag = false;
volatile uint8_t g_rx_char = 0;
volatile bool    g_rx_flag = false;
static char send_buff[128];

/* 初始化 UART */
void UART_Init(void)
{
    R_SCI_UART_Open(&g_uart9_ctrl, &g_uart9_cfg);
    R_SCI_UART_Open(&g_uart4_ctrl, &g_uart4_cfg);
}

/* FSP 自动生成的 UART 回调函数 */
void uart9_callback (uart_callback_args_t * p_args)
{
    if(p_args->event == UART_EVENT_TX_COMPLETE)
    {
        uart_send_complete_flag = true;
    }
    else if(p_args->event == UART_EVENT_RX_CHAR)
    {
        g_rx_char = (uint8_t)p_args->data;
        g_rx_flag = true;
    }
}

void uart4_callback (uart_callback_args_t * p_args)
{
    if(p_args->event == UART_EVENT_TX_COMPLETE)
    {
        uart_send_complete_flag = true;
    }
    else if(p_args->event == UART_EVENT_RX_CHAR)
    {
        g_rx_char = (uint8_t)p_args->data;
        g_rx_flag = true;
    }
}


/* 格式化打印函数 */
void UART_Printf(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    uint32_t len = (uint32_t)vsnprintf(send_buff, sizeof(send_buff), format, args);
    va_end(args);

    uart_send_complete_flag = false;
    R_SCI_UART_Write(&g_uart9_ctrl, (uint8_t *)send_buff, len);
    // 等待发送完成，防止 buffer 覆盖
    while(uart_send_complete_flag == false);
}
