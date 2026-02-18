#include "hal_data.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ==========================================
 * 1. 宏定义与地址配置
 * ========================================== */
#define CAM_DEFAULT_I2C_ADDRESS       (0x34)
#define MOTOR_TYPE_ADDR               0x14
#define MOTOR_FIXED_SPEED_ADDR        0x33
#define MOTOR_ENCODER_POLARITY_ADDR   0x15
#define MOTOR_ENCODER_TOTAL_ADDR      0x3C
#define ADC_BAT_ADDR                  0x00

#define MOTOR_TYPE_JGB37_520_12V_110RPM   3

/* 速度定义 (可调整 0-100) */
#define SPEED_FAST     50
#define SPEED_STOP     0

/* 蓝牙 HEX 指令 (手机请用十六进制发送) */
#define CMD_STOP        0x00
#define CMD_FORWARD     0x01
#define CMD_BACKWARD    0x02
#define CMD_SLIDE_LEFT  0x03
#define CMD_SLIDE_RIGHT 0x04
#define CMD_TURN_LEFT   0x05
#define CMD_TURN_RIGHT  0x06
#define CMD_STOP_CMD    0x07

/* ==========================================
 * 2. 全局变量
 * ========================================== */
fsp_err_t err = FSP_SUCCESS;

volatile bool uart_send_complete_flag = false;
volatile bool i2c_operation_complete = false;

/* 接收缓冲区变量 */
volatile uint8_t g_rx_char = 0;
volatile bool    g_rx_flag = false;

char send_buff[128];

/* ==========================================
 * 3. 回调函数 (核心：同时处理发送和接收)
 * ========================================== */
void user_uart_callback (uart_callback_args_t * p_args)
{
    /* 发送完成 */
    if(p_args->event == UART_EVENT_TX_COMPLETE)
    {
        uart_send_complete_flag = true;
    }
    /* 接收到字符 (关键！需要FSP里开启接收中断) */
    else if(p_args->event == UART_EVENT_RX_CHAR)
    {
        g_rx_char = (uint8_t)p_args->data;
        g_rx_flag = true;
    }
}

void i2c_master_callback(i2c_master_callback_args_t * p_args)
{
    if (I2C_MASTER_EVENT_ABORTED == p_args->event) {
        i2c_operation_complete = true;
    } else if (I2C_MASTER_EVENT_RX_COMPLETE == p_args->event ||
               I2C_MASTER_EVENT_TX_COMPLETE == p_args->event) {
        i2c_operation_complete = true;
    }
}

/* ==========================================
 * 4. 辅助函数
 * ========================================== */
void UART_Printf(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    uint32_t len = (uint32_t)vsnprintf(send_buff, sizeof(send_buff), format, args);
    va_end(args);

    uart_send_complete_flag = false;
    R_SCI_UART_Write(&g_uart9_ctrl, (uint8_t *)send_buff, len);
    while(uart_send_complete_flag == false);
}

void I2C_Write_Reg(uint8_t reg, uint8_t * data, uint8_t len)
{
    uint8_t buf[32];
    buf[0] = reg;
    memcpy(&buf[1], data, len);

    i2c_operation_complete = false;
    R_IIC_MASTER_Write(&g_i2c_master0_ctrl, buf, len + 1, false);

    int timeout = 0;
    while(i2c_operation_complete == false && timeout < 1000000) { timeout++; }
}

/* ==========================================
 * 5. 麦克纳姆轮运动逻辑
 * M1:左前  M2:右前  M3:左后  M4:右后
 * ========================================== */
void Motor_Set4Speed(int8_t m1, int8_t m2, int8_t m3, int8_t m4)
{
    int8_t speed_data[4] = {m1, m2, m3, m4};
    I2C_Write_Reg(MOTOR_FIXED_SPEED_ADDR, (uint8_t *)speed_data, 4);
}

/* 动作封装 */
void Car_Stop(void) {
    Motor_Set4Speed(0, 0, 0, 0);
    UART_Printf("State: Stop\r\n");
}
void Car_Forward(void) {
    Motor_Set4Speed(SPEED_FAST, SPEED_FAST, SPEED_FAST, SPEED_FAST);
    UART_Printf("State: Forward\r\n");
}
void Car_Backward(void) {
    Motor_Set4Speed(-SPEED_FAST, -SPEED_FAST, -SPEED_FAST, -SPEED_FAST);
    UART_Printf("State: Back\r\n");
}
void Car_Slide_Left(void) {
    // 左平移: FL-, FR+, RL+, RR-
    Motor_Set4Speed(-SPEED_FAST, SPEED_FAST, SPEED_FAST, -SPEED_FAST);
    UART_Printf("State: Left Slide\r\n");
}
void Car_Slide_Right(void) {
    // 右平移: FL+, FR-, RL-, RR+
    Motor_Set4Speed(SPEED_FAST, -SPEED_FAST, -SPEED_FAST, SPEED_FAST);
    UART_Printf("State: Right Slide\r\n");
}
void Car_Turn_Left(void) {
    // 左转: 左-, 右+
    Motor_Set4Speed(-SPEED_FAST, SPEED_FAST, -SPEED_FAST, SPEED_FAST);
    UART_Printf("State: Turn Left\r\n");
}
void Car_Turn_Right(void) {
    // 右转: 左+, 右-
    Motor_Set4Speed(SPEED_FAST, -SPEED_FAST, SPEED_FAST, -SPEED_FAST);
    UART_Printf("State: Turn Right\r\n");
}

/* ==========================================
 * 6. 主程序
 * ========================================== */
void hal_entry(void)
{
    /* 初始化 */
    R_SCI_UART_Open(&g_uart9_ctrl, &g_uart9_cfg);

    UART_Printf("\r\n============================\r\n");
    UART_Printf(">>> 1. System Power On! UART OK <<<\r\n");
    UART_Printf("==================="
            "=========\r\n");
    R_IIC_MASTER_Open(&g_i2c_master0_ctrl, &g_i2c_master0_cfg);
    R_IIC_MASTER_SlaveAddressSet(&g_i2c_master0_ctrl, CAM_DEFAULT_I2C_ADDRESS, I2C_MASTER_ADDR_MODE_7BIT);

    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    UART_Printf("Bluetooth Control Ready! Send Hex: 01-07\r\n");

    /* 配置电机 */
    uint8_t type = MOTOR_TYPE_JGB37_520_12V_110RPM;
    I2C_Write_Reg(MOTOR_TYPE_ADDR, &type, 1);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    uint8_t polarity = 0;
    I2C_Write_Reg(MOTOR_ENCODER_POLARITY_ADDR, &polarity, 1);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    while(1)
    {
        /* 检查是否有新命令 */
        if(g_rx_flag)
        {
            uint8_t cmd = g_rx_char;
            g_rx_flag = false; // 清除标志

            switch(cmd)
            {
                case CMD_FORWARD:     Car_Forward();     break; // 0x01
                case CMD_BACKWARD:    Car_Backward();    break; // 0x02
                case CMD_SLIDE_LEFT:  Car_Slide_Left();  break; // 0x03
                case CMD_SLIDE_RIGHT: Car_Slide_Right(); break; // 0x04
                case CMD_TURN_LEFT:   Car_Turn_Left();   break; // 0x05
                case CMD_TURN_RIGHT:  Car_Turn_Right();  break; // 0x06
                case CMD_STOP_CMD:
                case CMD_STOP:        Car_Stop();        break; // 0x07 or 0x00

                default:
                    // 调试神器：如果你发错了格式(如发了ASCII)，这里会告诉你发了什么
                    UART_Printf("Unknown Hex: 0x%02X\r\n", cmd);
                    break;
            }
        }
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
    }
}

/* ==========================================
 * 7. 必要的 WarmStart (你验证过它是必须的)
 * ========================================== */
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
