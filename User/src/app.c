#include "app.h"

void app_init(void)
{
	HAL_TIM_Base_Start_IT(&htim1);
	
	brt38_encoder_init();
}

void app_run(void)
{
	while (1) {
		
	}
}
