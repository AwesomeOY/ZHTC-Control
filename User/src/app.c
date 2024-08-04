#include "app.h"
#include "motor.h"
#include "cmsis_os2.h"
#include "valve_control.h"

uint8_t key_status = 0;    // 0-空,1-按下,2-释放
uint32_t key_increment = 0;
valve_control_obj valve_input, valve_output; // 进水电磁阀
float motor_spd = 0.0f;
float target_distance = 0.0f;
uint8_t sys_start = 0;
osEventFlagsId_t collect_event;

void app_init(void)
{
	osEventFlagsAttr_t event = {"COLLECT_EVENT", 0, NULL, 0};
	collect_event = osEventFlagsNew(&event);
	
	HAL_TIM_Base_Start_IT(&htim1);
	
	// 所有电平转换芯片有效
	gpio_level_output_enable();
	
	collect_system_init();
	
	collect_task_init();
	
	mavlink_init();
	
	brt38_encoder_init();
	
	motor_init();
	
	param_sensor_update_task_init();
	
	usb_task_init();
}

void app_run(void)
{
	//param_sensor_start();
	//gpio_auto_test();
}
