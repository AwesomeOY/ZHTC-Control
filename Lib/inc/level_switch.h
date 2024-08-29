/**
*** 液位反馈开关接口
**/

#ifndef LEVEL_SWITCH_H_
#define LEVEL_SWITCH_H_

#include "userdef.h"

typedef enum {
	LEVEL_SWITCH_STATUS_CLOSE,
	LEVEL_SWITCH_STATUS_OPEN
}LEVEL_SWITCH_STATUS;

typedef struct {
	Pin pin;
	uint8_t valid_level;  // 有效电平，0-高，1-低
}level_switch_input;

void level_switch_input_init(level_switch_input* pls, Pin input_pin, uint8_t level);

LEVEL_SWITCH_STATUS level_switch_input_read(level_switch_input* pls);

#endif
