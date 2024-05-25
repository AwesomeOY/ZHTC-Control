/**
*** 电机控制接口
**/

#ifndef MOTOR_H_
#define MOTOR_H_

#include "userdef.h"
#include "main.h"

#define water_pump_open()  WRITE_PIN(WATER_PUMP_GPIO_Port, WATER_PUMP_Pin, GPIO_PIN_SET)
#define water_pump_close() WRITE_PIN(WATER_PUMP_GPIO_Port, WATER_PUMP_Pin, GPIO_PIN_RESET)

#define motor_push() WRITE_PIN(MOTOR_DIC_GPIO_Port, MOTOR_DIC_Pin, GPIO_PIN_SET); WRITE_PIN(MOTOR_SPEED_GPIO_Port, MOTOR_SPEED_Pin, GPIO_PIN_RESET);
#define motor_pull() WRITE_PIN(MOTOR_DIC_GPIO_Port, MOTOR_DIC_Pin, GPIO_PIN_RESET); WRITE_PIN(MOTOR_SPEED_GPIO_Port, MOTOR_SPEED_Pin, GPIO_PIN_RESET);
#define motor_stop() WRITE_PIN(MOTOR_SPEED_GPIO_Port, MOTOR_SPEED_Pin, GPIO_PIN_SET)

void motor_set_position(float pos);

void motor_set_speed(float spd);

#endif
