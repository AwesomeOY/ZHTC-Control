#ifndef APP_H_
#define APP_H_

#include "main.h"
#include "brt38_encoder.h"
#include "userdef.h"

extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

#define sys_run_led_toggle() HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin)

#define key_is_down() 	 (READ_PIN(KEY_IN_GPIO_Port, KEY_IN_Pin) == GPIO_PIN_SET)
#define key_is_release() (READ_PIN(KEY_IN_GPIO_Port, KEY_IN_Pin) == GPIO_PIN_RESET)

void app_init(void);

void app_run(void);

#endif
