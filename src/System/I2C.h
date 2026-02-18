#ifndef I2C_H
#define I2C_H

#include "hal_data.h"
#include <string.h>

/* 这里假设一些常用地址，根据实际硬件调整 */
#define CAM_DEFAULT_I2C_ADDRESS 0x34 // 举例

extern volatile bool i2c_operation_complete;

void I2C_Init(void);
void I2C_Write_Reg(uint8_t reg, uint8_t * data, uint8_t len);

#endif
