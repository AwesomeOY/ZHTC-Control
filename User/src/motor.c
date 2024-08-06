#include "motor.h"
#include <math.h>

extern TIM_HandleTypeDef htim4;
float _target_pos;
uint8_t _need_update = 0;

void motor_init(void)
{
	motor_power_off();
	htim4.Instance->CCR2 = 0;
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	motor_power_on();
}

void motor_set_position(float pos)
{
	_target_pos = pos;
	_need_update = 1;
}

float motor_get_target_pos(void)
{
	return _target_pos;
}

uint8_t motor_need_control(void)
{
	return _need_update;
}

void motor_set_speed(float spd)
{
	if (spd <= 0.001f && spd >= -0.001f ) {
		spd = 0.0f;
	}
	if (spd < 0.0f) {
		motor_pull();
	} else {
		motor_push();
	}
	spd = fabs(spd);
	spd = spd * (0.8f-0.3f) + 0.4f;
	if (spd <= 0.40f) {
		htim4.Instance->CCR2 = 0;
	} else {
		htim4.Instance->CCR2 = (uint16_t)(MOTOR_SPD_PWM_MAX * spd);
	}
}

void motor_stop(void)
{
	htim4.Instance->CCR2 = 0;
	_need_update = 0;
}
