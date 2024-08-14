#ifndef PID_H_
#define PID_H_

#include <stdint.h>

typedef struct {
	float kp;
	float ki;
	float kd;
	float target;
	float result;
	float pre_error;
	float pre_pre_error;
}PidInc;

void pid_inc_init(PidInc* ppid, float kp, float ki, float kd);

float pid_inc_update(PidInc* ppid, float target, float current);

#endif
