#ifndef APP_H_
#define APP_H_

#include "main.h"
#include "brt38_encoder.h"

extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

void app_init(void);

void app_run(void);

#endif
