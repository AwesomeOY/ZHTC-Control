#include "motor.h"
#include <math.h>

extern TIM_HandleTypeDef htim4;

void motor_init(void)
{
	motor_power_off();
	htim4.Instance->CCR2 = MOTOR_SPD_PWM_MAX + 5;
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	motor_power_on();
}

void motor_set_position(float pos)
{

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
	spd = spd >= 0.8f ? 0.8f : 0;
	htim4.Instance->CCR2 = (uint16_t)(MOTOR_SPD_PWM_MAX * spd);
}

void motor_close(void)
{
	
	htim4.Instance->CCR2 = MOTOR_SPD_PWM_MAX + 5;
}
