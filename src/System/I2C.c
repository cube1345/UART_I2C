#include "I2C.h"

volatile bool i2c_operation_complete = false;

void I2C_Init(void)
{
    R_IIC_MASTER_Open(&g_i2c_master0_ctrl, &g_i2c_master0_cfg);
    // 这里的 Slave Address 可能需要根据操作的设备动态修改，或者在 Open 后设置默认
    R_IIC_MASTER_SlaveAddressSet(&g_i2c_master0_ctrl, CAM_DEFAULT_I2C_ADDRESS, I2C_MASTER_ADDR_MODE_7BIT);
}

/* FSP 自动生成的 I2C 回调函数 */
void i2c_master_callback(i2c_master_callback_args_t * p_args)
{
    if (I2C_MASTER_EVENT_ABORTED == p_args->event) {
        i2c_operation_complete = true;
    } else if (I2C_MASTER_EVENT_RX_COMPLETE == p_args->event ||
               I2C_MASTER_EVENT_TX_COMPLETE == p_args->event) {
        i2c_operation_complete = true;
    }
}

void I2C_Write_Reg(uint8_t reg, uint8_t * data, uint8_t len)
{
    uint8_t buf[32]; // 注意数据长度不要溢出
    buf[0] = reg;
    if(len > 0 && data != NULL)
    {
        memcpy(&buf[1], data, len);
    }

    i2c_operation_complete = false;
    R_IIC_MASTER_Write(&g_i2c_master0_ctrl, buf, len + 1, false);

    /* 简单的超时等待 */
    int timeout = 0;
    while(i2c_operation_complete == false && timeout < 1000000) { timeout++; }
}
