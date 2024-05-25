#ifndef VALVE_CONTROL_H_
#define VALVE_CONTROL_H_

#include <stdint.h>
#include "userdef.h"

typedef enum {
	VALVE_STATUS_NONE = 0,      // 初始化状态		
	VALVE_STATUS_OPEN_ING,      // 开启中
	VALVE_STATUS_OPEN_SUCCESS,  // 成功开启
	VALVE_STATUS_CLOSE_ING,     // 关闭中
	VALVE_STATUS_CLOSE_SUCCESS  // 成功关闭
}VALVE_STATUS_ENUM;

typedef enum {
	VALVE_ERROR_CODE_OK = 0, // 开/关正常
	VALVE_ERROR_CODE_OPEN,   // 开启超时错误
	VALVE_ERROR_CODE_CLOSE,  // 关闭超时错误
	VALVE_ERROR_CODE_SIGNAL, // 反馈信号错误: 同高或者同低电平(要么式开启，要么是关闭)
}VALVE_ERROR_CODE;

typedef struct {
	VALVE_STATUS_ENUM run_status;   // 运行中标志
	VALVE_ERROR_CODE error_status;  // 错误标志
	Pin open_control_pin;       // 开关控制引脚
	Pin open_success_pin;       // 开到位输入引脚
	Pin close_success_pin;      // 关到位输入引脚
	__IO uint32_t start_time_ms;// 开启动作时刻时间，用户超时错误判断
}valve_control_obj;

void valve_control_init(valve_control_obj* pvc, Pin control_pin, Pin open_status_pin, Pin close_status_pin);

void valve_open(valve_control_obj* pvc);
	
void valve_close(valve_control_obj* pvc);

VALVE_STATUS_ENUM valve_status(valve_control_obj* pvc);

VALVE_ERROR_CODE valve_error_status(valve_control_obj* pvc);

void valve_status_update(valve_control_obj* pvc);

#endif
