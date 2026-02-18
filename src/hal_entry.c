#include "hal_data.h"
#include "UART.h"
#include "I2C.h"
#include "MotorModule.h"
#include "Car.h"
#include "BlueTooth.h"

void hal_entry(void)
{
    /* 1. 初始化 UART */
    UART_Init();

    UART_Printf("\r\n============================\r\n");
    UART_Printf(">>> System Power On! Modules Init... <<<\r\n");
    UART_Printf("============================\r\n");

    /* 2. 初始化 I2C */
    I2C_Init();
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    /* 3. 初始化电机 */
    UART_Printf("Init Motors...\r\n");
    Motor_Init();

    UART_Printf("Bluetooth Control Ready! Send Hex: 01-07\r\n");

    while(1)
    {
        /* 检查是否有新命令 (来自 UART 模块的全局变量) */
        if(g_rx_flag)
        {
            g_rx_flag = false; // 清除标志
            uint8_t cmd = g_rx_char;
            switch(cmd)
            {
                case CMD_FORWARD:     Car_Forward();     break;
                case CMD_BACKWARD:    Car_Backward();    break;
                case CMD_SLIDE_LEFT:  Car_Slide_Left();  break;
                case CMD_SLIDE_RIGHT: Car_Slide_Right(); break;
                case CMD_TURN_LEFT:   Car_Turn_Left();   break;
                case CMD_TURN_RIGHT:  Car_Turn_Right();  break;
                case CMD_STOP_CMD:
                case CMD_STOP:        Car_Stop();        break;

                default:
                    UART_Printf("Unknown Hex: 0x%02X\r\n", cmd);
                    break;
            }
}


        }
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
    }


void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0
        R_FACI_LP->DFLCTL = 1U;
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        R_IOPORT_Open (&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);
#if BSP_CFG_SDRAM_ENABLED
        R_BSP_SdramInit(true);
#endif
    }
}
