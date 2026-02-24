#include "GimbalControl.h"

static float current_tilt_pwm = PWM_MID;

void Gimbal_Init(void) {
    R_GPT_Open(&g_timer_pan_ctrl, &g_timer_pan_cfg);
    R_GPT_Start(&g_timer_pan_ctrl);
    R_GPT_Open(&g_timer_tilt_ctrl, &g_timer_tilt_cfg);
    R_GPT_Start(&g_timer_tilt_ctrl);

    // 修正：增加了第三个参数 GPT_IO_PIN_GTIOCA
    R_GPT_DutyCycleSet(&g_timer_pan_ctrl, PWM_MID, GPT_IO_PIN_GTIOCA);
    R_GPT_DutyCycleSet(&g_timer_tilt_ctrl, PWM_MID, GPT_IO_PIN_GTIOCA);
}

void Gimbal_Set_Pan(float pid_out) {
    int32_t duty = PWM_MID + (int32_t)pid_out;
    if (duty > PWM_MAX) duty = PWM_MAX;
    if (duty < PWM_MIN) duty = PWM_MIN;

    // 死区：PID输出绝对值小于1500计数时不转（约15微秒偏移）
    if (pid_out > -1500 && pid_out < 1500) duty = PWM_MID;

    R_GPT_DutyCycleSet(&g_timer_pan_ctrl, (uint32_t)duty, GPT_IO_PIN_GTIOCA);
}

void Gimbal_Set_Tilt(float pid_out) {
    current_tilt_pwm += pid_out;
    if (current_tilt_pwm > PWM_MAX) current_tilt_pwm = PWM_MAX;
    if (current_tilt_pwm < PWM_MIN) current_tilt_pwm = PWM_MIN;

    R_GPT_DutyCycleSet(&g_timer_tilt_ctrl, (uint32_t)current_tilt_pwm, GPT_IO_PIN_GTIOCA);
}
