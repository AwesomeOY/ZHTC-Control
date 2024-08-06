/**
*** 电机控制接口
**/

#ifndef MOTOR_H_
#define MOTOR_H_

#include "userdef.h"
#include "main.h"

#define MOTOR_SPD_PWM_MAX 4200U

// 调速电机，驱动绞盘
#define motor_push() WRITE_PIN(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)
#define motor_pull() WRITE_PIN(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)
#define motor_power_on() WRITE_PIN(GPIOE, GPIO_PIN_11, GPIO_PIN_SET)
#define motor_power_off() WRITE_PIN(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET)

void motor_init(void);

void motor_set_position(float pos);

void motor_set_speed(float spd);

void motor_stop(void);

uint8_t motor_need_control(void);

float motor_get_target_pos(void);

#endif
