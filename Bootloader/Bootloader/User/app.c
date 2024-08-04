#include "app.h"
#include "cmsis_os2.h"

uint32_t led_toggle_time_ms = 200;

void app_init(void)
{	
	usb_task_init();
}

void app_run(void)
{
	//param_sensor_start();
}
