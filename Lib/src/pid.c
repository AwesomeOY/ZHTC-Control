#include "pid.h"

void pid_inc_init(PidInc* ppid, float kp, float ki, float kd)
{
	ppid->kp = kp;
	ppid->ki = ki;
	ppid->kd = kd;
	ppid->target = 0.0f;
	ppid->pre_error = 0.0f;
	ppid->pre_pre_error = 0.0f;
	ppid->result = 0.0f;
}

float pid_inc_update(PidInc* ppid, float target, float current)
{
	float error = target - current;
	ppid->target = target;
	ppid->result += ppid->kp * (error - ppid->pre_error);
	ppid->result += ppid->ki * error;
	ppid->result += ppid->kd * (error - 2 * ppid->pre_error + ppid->pre_pre_error);
	ppid->pre_pre_error = ppid->pre_error;
	ppid->pre_error = error;
	if (ppid->result >= 0.5f) {
		ppid->result = 0.5f;
	} else if (ppid->result <= -0.5f) {
		ppid->result = -0.5f;
	}
	return ppid->result;
}
