#include "app.h"
#include "ParamSensor5.h"
extern osEventFlagsId_t collect_event;

osThreadId_t paramSensorTaskHandle;
const osThreadAttr_t paramSensorTask_attributes = {
  .name = "Sensor_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t)osPriorityNormal7,
};

static osSemaphoreId_t _param_sensor_start_sem;

/* 应用层调用开启数据采集任务 */
void param_sensor_start(void)
{
	osSemaphoreRelease(_param_sensor_start_sem);
}

void param_sensor_update_task_init(void)
{
	param_sensor_init();
	osSemaphoreAttr_t attr = { NULL,  0, NULL };
	_param_sensor_start_sem = osSemaphoreNew(1, 0, &attr);
	osThreadNew(param_sensor_task, NULL, &paramSensorTask_attributes);
}

/* 多参数池数据更新任务，由采水任务到数据测量阶段才开启 */
void param_sensor_task(void* arg)
{
	uint16_t count = 0;
	while (1) {
		if (osOK == osSemaphoreAcquire(_param_sensor_start_sem, 0xffffffffUL)) {
			param_sensor_restart();
			osDelay(60000u);
			while (1) {
				const ParamSensor5* pparam5 = NULL;
				const ParamSensor4* pparam4 = NULL;
				param_sensor_update();
				pparam5 = get_current_param_sensor5();  // 更新多参数参数
				pparam4 = get_current_param_sensor4();  // 更新多参数参数
				if (pparam5->success && pparam4->success) {
					count = 0;
					osEventFlagsSet(collect_event, PARAM_SENSOR_SUCCESS_EVENT_BIT);
					break;
				}
//				++count;
//				if (count >= 200u) {
//					count = 0;
//					osEventFlagsSet(collect_event, PARAM_SENSOR_ERROR_EVENT_BIT);
//					break;
//				}
				osDelay(50);
			}
		}
	}
}
