#ifndef TRACKING_PID_H
#define TRACKING_PID_H

#include "hal_data.h"

typedef struct {
    float kp, ki, kd;
    float last_err;
    float integral;
} PID_Ctrl;

// 声明全局变量，供其他文件使用
extern PID_Ctrl pid_pan;
extern PID_Ctrl pid_tilt;

void PID_Init(void);
float PID_Compute(PID_Ctrl *p, float error);
bool K210_Protocol_Parse(uint8_t byte, int32_t *x, int32_t *y);

#endif
