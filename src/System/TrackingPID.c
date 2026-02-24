#include "TrackingPID.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PID_Ctrl pid_pan;
PID_Ctrl pid_tilt;

void PID_Init(void) {
    // 360舵机Kp：控制旋转速度
    pid_pan.kp = 100.0f; pid_pan.ki = 0.0f; pid_pan.kd = 10.0f;
    // 180舵机Kp：控制步进量
    pid_tilt.kp = 50.0f; pid_tilt.ki = 0.0f; pid_tilt.kd = 5.0f;
}

float PID_Compute(PID_Ctrl *p, float error) {
    float d_err = error - p->last_err;
    float out = (p->kp * error) + (p->kd * d_err);
    p->last_err = error;
    return out;
}

bool K210_Protocol_Parse(uint8_t byte, int32_t *x, int32_t *y) {
    static char buf[32];
    static int idx = 0;
    static bool start = false;

    if (byte == '#') { start = true; idx = 0; return false; }
    if (start) {
        if (byte == '*') {
            buf[idx] = '\0';
            start = false;
            char *comma = strchr(buf, ',');
            if (comma) {
                *comma = '\0';
                *x = atoi(buf);
                *y = atoi(comma + 1);
                return true;
            }
        } else {
            buf[idx++] = (char)byte;
            if (idx >= 31) start = false;
        }
    }
    return false;
}
