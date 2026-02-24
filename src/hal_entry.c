#include "hal_data.h"
#include "UART.h"
#include "GimbalControl.h"
#include "TrackingPID.h"
#include "MotorModule.h"
#include "Car.h"

// 引用外部变量
extern volatile int32_t k210_x;
extern volatile int32_t k210_y;
extern volatile bool    k210_ready;
extern PID_Ctrl pid_pan;
extern PID_Ctrl pid_tilt;

// 函数声明
void R_BSP_WarmStart(bsp_warm_start_event_t event);

void hal_entry(void)
{
    /* --- 1. 系统初始化与欢迎信息 --- */
    UART_Init();
    UART_Printf("\r\n***************************************\r\n");
    UART_Printf("*    RA4M2 Gimbal Tracking System     *\r\n");
    UART_Printf("*    Status: Initializing...          *\r\n");
    UART_Printf("***************************************\r\n");

    /* --- 2. 初始化各硬件模块 --- */
    UART_Printf("[1/4] Motors & I2C...");
    Motor_Init();
    UART_Printf(" OK!\r\n");

    UART_Printf("[2/4] Gimbal PWM...");
    Gimbal_Init();
    UART_Printf(" OK!\r\n");

    UART_Printf("[3/4] PID Algorithm...");
    PID_Init();
    UART_Printf(" OK!\r\n");

    UART_Printf("[4/4] UART Control Ready!\r\n");
    UART_Printf("\r\n>>> SYSTEM ONLINE <<<\r\n");

    while(1)
    {
        /* --- 追踪逻辑 (K210数据触发) --- */
        if(k210_ready)
        {
            k210_ready = false;

            // 计算 PID 输出
            float out_pan  = PID_Compute(&pid_pan, (float)k210_x);
            float out_tilt = PID_Compute(&pid_tilt, (float)k210_y);

            // 执行控制
            Gimbal_Set_Pan(out_pan);
            Gimbal_Set_Tilt(out_tilt);
        }

        /* --- 车辆控制逻辑 (蓝牙指令触发) --- */
        if(g_rx_flag)
        {
            g_rx_flag = false;
            uint8_t cmd = g_rx_char;
            UART_Printf("CMD: 0x%02X\r\n", cmd);

            switch(cmd)
            {
                case 0x01: Car_Forward();     break;
                case 0x02: Car_Backward();    break;
                case 0x03: Car_Slide_Left();  break;
                case 0x04: Car_Slide_Right(); break;
                case 0x05: Car_Turn_Left();   break;
                case 0x06: Car_Turn_Right();  break;
                case 0x07: Car_Stop();        break;
                default:   UART_Printf("Unknown Command\r\n"); break;
            }
        }

        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    }
}

// 修正：提供 WarmStart 定义以消除警告
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
