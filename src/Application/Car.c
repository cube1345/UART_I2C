#include "Car.h"

/* * 麦克纳姆轮布局假设:
 * M1:左前  M2:右前
 * M3:左后  M4:右后
 */

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
