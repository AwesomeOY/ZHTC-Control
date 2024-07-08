#include "app.h"
#include "motor.h"
#include "cmsis_os2.h"
#include "valve_control.h"

uint8_t key_status = 0;    // 0-空,1-按下,2-释放
uint32_t key_increment = 0;
valve_control_obj valve_input, valve_output; // 进水电磁阀

static void key_process(void);
static void valve_init(void);

void app_init(void)
{
	HAL_TIM_Base_Start_IT(&htim1);
	
	mavlink_init();
	
	brt38_encoder_init();
	
	motor_init();
	
//	valve_init();
}

void app_run(void)
{
//	if (key_status != 1 && key_is_down()) {
//		osDelay(10);
//		if (key_is_down()) {;
//			key_status = 1;
//			sys_run_led_toggle();
//			key_process();
//		}
//	}
//	
//	if (key_status != 2 && key_is_release()) {
//		osDelay(10);
//		if (key_is_release()) {;
//			key_status = 2;
//		}
//	}
	
//	valve_status_update(&valve_input);
	
//	osDelay(1000);
//	WRITE_PIN(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
//	WRITE_PIN(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//	
//	osDelay(1000);
//	WRITE_PIN(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
//	WRITE_PIN(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//	
	WRITE_PIN(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);
//	WRITE_PIN(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
//	WRITE_PIN(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
//	
//	osDelay(60000);
//	WRITE_PIN(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);
//	osDelay(3000);

//	motor_set_speed(0.1f);
//	osDelay(1000);
//	motor_set_speed(-0.1f);
//	osDelay(1000);
}

static void valve_init(void)
{
//	Pin ctrl_pin = { XY03K_CONTROL_Pin, XY03K_CONTROL_GPIO_Port};
//	Pin open_pin = { XY03K_OPEN_INPUT_Pin, XY03K_OPEN_INPUT_GPIO_Port};
//	Pin close_pin = { XY03K_CLOSE_INPUT_Pin, XY03K_CLOSE_INPUT_GPIO_Port}; 
//	valve_control_init(&valve_input, ctrl_pin, open_pin, close_pin);
}

static void key_process(void)
{
	switch (key_increment++) {
		case 0:
			//motor_push();
			break;
		case 1:
			//motor_stop();			
			break;
		case 2:
			//motor_pull();
			break;
		case 3:
			//motor_stop();
			break;
		case 4:
			valve_open(&valve_input);
			break;
		case 5:
			valve_close(&valve_input);
			break;
		case 6:
			//water_pump_open();
			break;
		case 7:
			//water_pump_close();
			break;
		case 8:
			key_increment = 0;
			break;
	}
}
