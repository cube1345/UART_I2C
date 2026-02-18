#ifndef CAR_H
#define CAR_H

#include "MotorModule.h"
#include "UART.h"

/* 速度定义 */
#define SPEED_FAST     50
#define SPEED_STOP     0

/* 动作函数 */
void Car_Stop(void);
void Car_Forward(void);
void Car_Backward(void);
void Car_Slide_Left(void);
void Car_Slide_Right(void);
void Car_Turn_Left(void);
void Car_Turn_Right(void);

#endif
