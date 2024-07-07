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

#define gpio_level_switch_on()  WRITE_PIN(GPIOD, GPIO_PIN_3, GPIO_PIN_SET)
#define gpio_level_switch_off() WRITE_PIN(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET)

#endif
