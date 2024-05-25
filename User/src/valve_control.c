#include "valve_control.h"

#define OPEN_LEVEL 	GPIO_PIN_SET
#define CLOSE_LEVEL	GPIO_PIN_RESET
#define VALID_LEVEL GPIO_PIN_SET

void valve_control_init(valve_control_obj* pvc, Pin control_pin, Pin open_status_pin, Pin close_status_pin)
{
	pvc->run_status = VALVE_STATUS_NONE;
	pvc->open_control_pin = control_pin;
	pvc->open_success_pin = open_status_pin;
	pvc->close_success_pin = close_status_pin;
}

void valve_open(valve_control_obj* pvc)
{
	if (pvc->run_status == VALVE_STATUS_OPEN_ING || pvc->run_status == VALVE_STATUS_OPEN_SUCCESS) {
		return;
	}
	WRITE_PIN(pvc->open_control_pin.port, pvc->open_control_pin.pin, OPEN_LEVEL);
	pvc->run_status = VALVE_STATUS_OPEN_ING;
	pvc->start_time_ms = get_system_time();
}
	
void valve_close(valve_control_obj* pvc)
{
	if (pvc->run_status == VALVE_STATUS_CLOSE_ING || pvc->run_status == VALVE_STATUS_CLOSE_SUCCESS) {
		return;
	}
	WRITE_PIN(pvc->open_control_pin.port, pvc->open_control_pin.pin, CLOSE_LEVEL);
	pvc->run_status = VALVE_STATUS_CLOSE_ING;
	pvc->start_time_ms = get_system_time();
}

inline VALVE_STATUS_ENUM valve_run_status(valve_control_obj* pvc)
{
	return pvc->run_status;
}

inline VALVE_ERROR_CODE valve_error_status(valve_control_obj* pvc)
{
	return pvc->error_status;
}


/* 建议上层应用定时调用该函数，以识别电磁阀当前状态 */
void valve_status_update(valve_control_obj* pvc)
{
	uint32_t now = get_system_time();
	uint8_t level = 0;
	
	if (READ_PIN(pvc->open_success_pin.port, pvc->open_success_pin.pin) == VALID_LEVEL) {
		level = 1;		
	}
	if (READ_PIN(pvc->close_success_pin.port, pvc->close_success_pin.pin) == VALID_LEVEL) {
		level |= 2;		
	}
	
	// 开和关只能存在一个，两个引脚电平反馈不能同时为高或低
	if (level == 0x01) {
		if (pvc->run_status == VALVE_STATUS_OPEN_ING) {
			pvc->run_status = VALVE_STATUS_OPEN_SUCCESS;
			pvc->error_status = VALVE_ERROR_CODE_OK;
		}
	} else if (level == 0x02) {
		if (pvc->run_status == VALVE_STATUS_CLOSE_ING) {
			pvc->run_status = VALVE_STATUS_CLOSE_SUCCESS;
			pvc->error_status = VALVE_ERROR_CODE_OK;
		}
	} else {
		pvc->error_status = VALVE_ERROR_CODE_SIGNAL;
	}
	
	// 开启或者关闭动作超时，则赋值错误标志
	if (now - pvc->start_time_ms >= 30000u) {
		if (pvc->run_status == VALVE_STATUS_OPEN_ING) {
			pvc->error_status = VALVE_ERROR_CODE_OPEN;
		} else if (pvc->run_status == VALVE_STATUS_CLOSE_ING) {
			pvc->error_status = VALVE_ERROR_CODE_CLOSE;
		}
	}
}
