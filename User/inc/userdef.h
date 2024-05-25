#ifndef USER_DEF_H_
#define USER_DEF_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

typedef struct {
	uint16_t pin;
	GPIO_TypeDef* port;	
}Pin;

#define get_system_time() osKernelGetTickCount()

#define READ_PIN(port, pin) 			HAL_GPIO_ReadPin(port, pin)
#define WRITE_PIN(port, pin, level) 	HAL_GPIO_WritePin(port, pin, (GPIO_PinState)(level))

#endif
