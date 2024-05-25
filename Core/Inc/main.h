/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SYS_LED_Pin GPIO_PIN_1
#define SYS_LED_GPIO_Port GPIOA
#define MOTOR_DIC_Pin GPIO_PIN_4
#define MOTOR_DIC_GPIO_Port GPIOC
#define WATER_PUMP_Pin GPIO_PIN_5
#define WATER_PUMP_GPIO_Port GPIOC
#define LEVEL_INPUT1_Pin GPIO_PIN_0
#define LEVEL_INPUT1_GPIO_Port GPIOB
#define LEVEL_INPUT2_Pin GPIO_PIN_1
#define LEVEL_INPUT2_GPIO_Port GPIOB
#define LEVEL_INPUT3_Pin GPIO_PIN_2
#define LEVEL_INPUT3_GPIO_Port GPIOB
#define LEVEL_INPUT4_Pin GPIO_PIN_7
#define LEVEL_INPUT4_GPIO_Port GPIOE
#define XY03K_OPEN_INPUT_Pin GPIO_PIN_8
#define XY03K_OPEN_INPUT_GPIO_Port GPIOE
#define XY03K_CLOSE_INPUT_Pin GPIO_PIN_9
#define XY03K_CLOSE_INPUT_GPIO_Port GPIOE
#define MOTOR_FB_INPUT_Pin GPIO_PIN_10
#define MOTOR_FB_INPUT_GPIO_Port GPIOE
#define RS485_EN3_Pin GPIO_PIN_12
#define RS485_EN3_GPIO_Port GPIOB
#define MOTOR_SPEED_Pin GPIO_PIN_8
#define MOTOR_SPEED_GPIO_Port GPIOA
#define RS485_EN1_Pin GPIO_PIN_11
#define RS485_EN1_GPIO_Port GPIOA
#define RS485_EN2_Pin GPIO_PIN_4
#define RS485_EN2_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
