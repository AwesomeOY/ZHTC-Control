#include "app.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim1) {
		static uint32_t count = 0;
		++count;
		if (count >= 20000) {
			//HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
			count = 0;
		}
	
	}
}

