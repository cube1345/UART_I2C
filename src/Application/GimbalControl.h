#ifndef GIMBAL_CONTROL_H
#define GIMBAL_CONTROL_H

#include "hal_data.h"

// 基于 FSP 中的 period_counts = 2,000,000 (20ms) 计算
#define PWM_PERIOD      2000000
#define PWM_MID         150000   // 1.5ms (停止/中位)
#define PWM_MIN         50000    // 0.5ms (最小)
#define PWM_MAX         250000   // 2.5ms (最大)

void Gimbal_Init(void);
void Gimbal_Set_Pan(float pid_out);
void Gimbal_Set_Tilt(float pid_out);

#endif
