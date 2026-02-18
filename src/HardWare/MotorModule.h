#ifndef MOTOR_MODULE_H
#define MOTOR_MODULE_H

#include "hal_data.h"
#include "I2C.h"

/* 电机驱动板寄存器定义 (假设值，需确认) */
#define MOTOR_TYPE_ADDR             0x14
#define MOTOR_ENCODER_POLARITY_ADDR 0x15
#define MOTOR_FIXED_SPEED_ADDR      0x33 // 假设值，请根据手册修改

/* 电机类型定义 */
#define MOTOR_TYPE_JGB37_520_12V_110RPM 3 // 假设枚举值

void Motor_Init(void);
void Motor_Set4Speed(int8_t m1, int8_t m2, int8_t m3, int8_t m4);

#endif
