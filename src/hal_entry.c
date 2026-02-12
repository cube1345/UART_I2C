#include "hal_data.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h> // 【新增】必须包含这个头文件，否则 va_list 会报错

/* ==========================================
 * 配置宏定义
 * ========================================== */
/* I2C 地址 (7位地址，如果STM32用的是0x68，这里就是0x34) */
#define CAM_DEFAULT_I2C_ADDRESS       (0x34)

/* 寄存器地址 */
#define MOTOR_TYPE_ADDR               0x14
#define MOTOR_FIXED_SPEED_ADDR        0x33
#define MOTOR_ENCODER_POLARITY_ADDR   0x15
#define MOTOR_ENCODER_TOTAL_ADDR      0x3C
#define ADC_BAT_ADDR                  0x00

/* 电机参数 */
#define MOTOR_TYPE_JGB37_520_12V_110RPM   3

/* ==========================================
 * 全局变量
 * ========================================== */
fsp_err_t err = FSP_SUCCESS;

/* 标志位必须加 volatile，防止编译器优化 */
volatile bool uart_send_complete_flag = false;
volatile bool i2c_operation_complete = false;
volatile bool i2c_error_flag = false;

/* 业务变量 */
int8_t p1[4] = {50, 50, 50, 50};            // 前进速度
int8_t p2[4] = {-20, -20, -20, -20};        // 后退速度
int8_t stop_speed[4] = {0, 0, 0, 0};        // 停止
int8_t EncodeReset[16] = {0};               // 清零缓冲
int32_t EncodeTotal[4];                     // 编码器数据
int8_t MotorEncoderPolarity = 0;
int8_t MotorType = MOTOR_TYPE_JGB37_520_12V_110RPM;

unsigned char data_adc[2];
char send_buff[128]; // 串口缓冲区

/* ==========================================
 * 回调函数
 * ========================================== */

/* 串口回调 */
void user_uart_callback (uart_callback_args_t * p_args)
{
    if(p_args->event == UART_EVENT_TX_COMPLETE)
    {
        uart_send_complete_flag = true;
    }
}

/* I2C 回调 (增强版) */
void i2c_master_callback(i2c_master_callback_args_t * p_args)
{
    if (I2C_MASTER_EVENT_ABORTED == p_args->event)
    {
        /* 如果发生错误(NACK等)，也置位完成，避免主循环卡死，但记录错误 */
        i2c_operation_complete = true;
        i2c_error_flag = true;
    }
    else if (I2C_MASTER_EVENT_RX_COMPLETE == p_args->event ||
             I2C_MASTER_EVENT_TX_COMPLETE == p_args->event)
    {
        i2c_operation_complete = true;
        i2c_error_flag = false;
    }
}

/* ==========================================
 * 辅助函数：封装串口发送
 * ========================================== */
void UART_Printf(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    // 使用 vsnprintf 格式化字符串
    uint32_t len = (uint32_t)vsnprintf(send_buff, sizeof(send_buff), format, args);
    va_end(args);

    uart_send_complete_flag = false;
    R_SCI_UART_Write(&g_uart9_ctrl, (uint8_t *)send_buff, len);
    // 等待发送完成
    while(uart_send_complete_flag == false);
}

/* ==========================================
 * 主程序
 * ========================================== */
void hal_entry(void)
{
    uint16_t v;
    uint8_t i2c_buf[32]; // 通用 I2C 缓冲区

    /* 1. 初始化串口 */
    err = R_SCI_UART_Open(&g_uart9_ctrl, &g_uart9_cfg);
    assert(FSP_SUCCESS == err);

    /* 2. 初始化 I2C */
    err = R_IIC_MASTER_Open(&g_i2c_master0_ctrl, &g_i2c_master0_cfg);
    assert(FSP_SUCCESS == err);

    /* 【关键】设置从机地址，防止 FSP 默认配置不是 0x34 */
    R_IIC_MASTER_SlaveAddressSet(&g_i2c_master0_ctrl, CAM_DEFAULT_I2C_ADDRESS, I2C_MASTER_ADDR_MODE_7BIT);

    /* 等待硬件稳定 */
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    // 启动提示
    UART_Printf("System Init OK. Motor Driver Test Start...\r\n");

    /* 3. 配置电机类型 */
    i2c_buf[0] = MOTOR_TYPE_ADDR;
    i2c_buf[1] = (uint8_t)MotorType;

    i2c_operation_complete = false;
    R_IIC_MASTER_Write(&g_i2c_master0_ctrl, i2c_buf, 2, false);
    while(i2c_operation_complete == false);

    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    /* 4. 配置电机极性 */
    i2c_buf[0] = MOTOR_ENCODER_POLARITY_ADDR;
    i2c_buf[1] = (uint8_t)MotorEncoderPolarity;

    i2c_operation_complete = false;
    R_IIC_MASTER_Write(&g_i2c_master0_ctrl, i2c_buf, 2, false);
    while(i2c_operation_complete == false);

    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    while(1)
    {
        /* ====================================================
         * A. 读取电池电压
         * ==================================================== */
        uint8_t reg_adc = ADC_BAT_ADDR;

        // 写寄存器地址 (Restart = true)
        i2c_operation_complete = false;
        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg_adc, 1, true);
        while(i2c_operation_complete == false);

        // 读取数据
        i2c_operation_complete = false;
        R_IIC_MASTER_Read(&g_i2c_master0_ctrl, data_adc, 2, false);
        while(i2c_operation_complete == false);

        if(!i2c_error_flag) {
            v = (uint16_t)(data_adc[0] | (data_adc[1] << 8));
            UART_Printf("Battery Voltage = %d mV\r\n", v);
        } else {
            UART_Printf("I2C Error Reading Voltage!\r\n");
        }

        /* ====================================================
         * B. 电机前进 (P1)
         * ==================================================== */
        i2c_buf[0] = MOTOR_FIXED_SPEED_ADDR;
        memcpy(&i2c_buf[1], p1, 4);

        i2c_operation_complete = false;
        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, i2c_buf, 5, false);
        while(i2c_operation_complete == false);

        UART_Printf("Motor Forward...\r\n");
        R_BSP_SoftwareDelay(3000, BSP_DELAY_UNITS_MILLISECONDS);

        /* ====================================================
         * C. 电机后退 (P2)
         * ==================================================== */
        i2c_buf[0] = MOTOR_FIXED_SPEED_ADDR;
        memcpy(&i2c_buf[1], p2, 4);

        i2c_operation_complete = false;
        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, i2c_buf, 5, false);
        while(i2c_operation_complete == false);

        UART_Printf("Motor Backward...\r\n");
        R_BSP_SoftwareDelay(3000, BSP_DELAY_UNITS_MILLISECONDS);

        /* ====================================================
         * D. 读取编码器总值
         * ==================================================== */
        uint8_t reg_enc = MOTOR_ENCODER_TOTAL_ADDR;

        i2c_operation_complete = false;
        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg_enc, 1, true);
        while(i2c_operation_complete == false);

        i2c_operation_complete = false;
        R_IIC_MASTER_Read(&g_i2c_master0_ctrl, (uint8_t *)EncodeTotal, 16, false);
        while(i2c_operation_complete == false);

        UART_Printf("Enc: %ld %ld %ld %ld\r\n", EncodeTotal[0], EncodeTotal[1], EncodeTotal[2], EncodeTotal[3]);

        /* ====================================================
         * E. 电机停止
         * ==================================================== */
        i2c_buf[0] = MOTOR_FIXED_SPEED_ADDR;
        memcpy(&i2c_buf[1], stop_speed, 4);

        i2c_operation_complete = false;
        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, i2c_buf, 5, false);
        while(i2c_operation_complete == false);

        UART_Printf("Motor Stop.\r\n");
        R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

        /* ====================================================
         * F. 清零编码器并验证 (之前出错的地方)
         * ==================================================== */

        // 1. 发送清零命令
        i2c_buf[0] = MOTOR_ENCODER_TOTAL_ADDR;
        memcpy(&i2c_buf[1], EncodeReset, 16);

        i2c_operation_complete = false;
        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, i2c_buf, 17, false);
        while(i2c_operation_complete == false);

        // 2. 发送读取命令 (注意：再次操作前必须清标志位！)
        i2c_operation_complete = false;
        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg_enc, 1, true);
        while(i2c_operation_complete == false);

        // 3. 读取数据 (注意：再次操作前必须清标志位！)
        i2c_operation_complete = false;
        R_IIC_MASTER_Read(&g_i2c_master0_ctrl, (uint8_t *)EncodeTotal, 16, false);
        while(i2c_operation_complete == false);

        UART_Printf("Reset OK! Enc1 now: %ld\r\n", EncodeTotal[0]);

        R_BSP_SoftwareDelay(3000, BSP_DELAY_UNITS_MILLISECONDS);
    }
}


/* 将这段代码复制到 hal_entry.c 的最底部（在 hal_entry 函数的大括号外面） */

void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0
        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        /* 关键：这就话负责把引脚从 GPIO 切换为 UART/I2C 功能 */
        R_IOPORT_Open (&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);

#if BSP_CFG_SDRAM_ENABLED
        /* Setup SDRAM and initialize it. Must configure pins first. */
        R_BSP_SdramInit(true);
#endif
    }
}
