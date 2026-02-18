#include "MotorModule.h"

void Motor_Init(void)
{
    /* 配置电机类型 */
    uint8_t type = MOTOR_TYPE_JGB37_520_12V_110RPM;
    I2C_Write_Reg(MOTOR_TYPE_ADDR, &type, 1);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    /* 配置极性 */
    uint8_t polarity = 0;
    I2C_Write_Reg(MOTOR_ENCODER_POLARITY_ADDR, &polarity, 1);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
}

void Motor_Set4Speed(int8_t m1, int8_t m2, int8_t m3, int8_t m4)
{
    int8_t speed_data[4] = {m1, m2, m3, m4};
    I2C_Write_Reg(MOTOR_FIXED_SPEED_ADDR, (uint8_t *)speed_data, 4);
}
