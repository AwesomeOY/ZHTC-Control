#ifndef APP_H_
#define APP_H_

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "brt38_encoder.h"
#include "userdef.h"
#include "serial.h"
#include "ByteQueue.h"
#include "mavlink.h"
#include "motor.h"

typedef enum {
	COLLECT_TASK_CMD_NONE = 0,            // 无动作
	COLLECT_TASK_CMD_START_GET_WATER = 1, // 开始取水
	COLLECT_TASK_CMD_PAUSE_GET_WATER = 2, // 暂停取水
	COLLECT_TASK_CMD_RESUME_GET_WATER = 3,// 恢复取水
	COLLECT_TASK_CMD_BOTTLE_RESET = 5,    // 瓶状态复位
	COLLECT_TASK_CMD_CLEAN_PIPE = 6,      // 清洗管路
	COLLECT_TASK_CMD_CLOSE_PIPE = 7,      // 关闭管路
	COLLECT_TASK_CMD_START_PURGE_PIPE = 8,// 吹洗管路
	COLLECT_TASK_CMD_STOP_PURGE_PIPE = 9, // 关闭吹洗管路
	COLLECT_TASK_CMD_MANUAL_PUSH_PIPE = 10,      // 抽水管手动下降
	COLLECT_TASK_CMD_MANUAL_PULL_PIPE = 11,      // 抽水管手动上升          
	COLLECT_TASK_CMD_MANUAL_PAUSE_PUSH_PULL = 12, // 抽水管暂停上升或者下降   
}COLLECT_TASK_CMD;

typedef enum {
	COLLECT_TASK_STATUS_IDLE = 0,           // 等待控制指令
	COLLECT_TASK_STATUS_PAUSE = 1, 			// 暂停
	COLLECT_TASK_STATUS_PUSH_PIPE = 2,      // 采水管下降
	COLLECT_TASK_STATUS_CLEANING = 3,       // 润洗
	COLLECT_TASK_STATUS_PUMP_WATER = 4,     // 泵吸采水
	COLLECT_TASK_STATUS_PULL_PIPE = 5,      // 采水管上升
	COLLECT_TASK_STATUS_CLEAN_BUS = 6,      // 排空汇流排
	COLLECT_TASK_STATUS_CLEAN_TREE = 7,     // 排空支路
	COLLECT_TASK_STATUS_CLOSE_FAN = 8,      // 关闭阀门
	COLLECT_TASK_STATUS_SUCCESS = 10,       // 采水完成
	COLLECT_TASK_STATUS_CLEAN_PIPE = 11,    // 清洗管路
	COLLECT_TASK_STATUS_FAN_CLEAN_PIPE = 12,  // 吹洗管路  	
	COLLECT_TASK_STATUS_MANUAL_PUSH_PIPE = 13,// 水管手动下降
	COLLECT_TASK_STATUS_MANUAL_PULL_PIPE = 14,// 水管手动上升         
	COLLECT_TASK_STATUS_MANUAL_PAUSE_PUSH_PULL = 15,// 水管暂停   
}COLLECT_TASK_STATUS;

typedef enum {
	COLLECT_MODE_NONE, 
	COLLECT_MODE_AUTO, 	// 全自主模式
	COLLECT_MODE_MANUAL // 手动模式
}COLLECT_MODE;

typedef struct {
	uint8_t header1;  // 固定为0xFB
	uint8_t header2;  // 固定为0xFB
	uint8_t header3;  // 固定为0xFE
	uint8_t cmd_dic;  // 固定为0x01
	uint8_t cmd;      // 对应COLLECT_TASK_CMD
	uint8_t bottle_num; // 本次采样使用的瓶编号 1 ~ 6
	uint8_t depth_dm; // 本次采样的深度 dm
	uint8_t res[9];   // 保留
}collect_cmd;

typedef struct {
	uint8_t header1;  // 固定为0xFB
	uint8_t header2;  // 固定为0xFB
	uint8_t header3;  // 固定为0xFE
	uint8_t cmd_dic;  // 固定为0x00
	uint8_t status;   // 当前取水状态
	uint8_t valve_status_l; // 阀门状态低八位
	uint8_t valve_status_h; // 阀门状态高八位
	uint8_t current_depth;  // 当前取水深度 dm
	uint8_t bottle_status;  // 所有采样瓶的状态，从低三位开始表示瓶的状态 0 - 空，1-满
	uint8_t res[7];
}collect_status;

extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

#define START_EVENT_BIT          (1<<0)
#define ERROR_EVENT_BIT          (1<<1)
#define EXIT_EVENT_BIT           (1<<2)
#define PARAM_SENSOR_SUCCESS_EVENT_BIT (1<<3)
#define PARAM_SENSOR_ERROR_EVENT_BIT   (1<<4)

#define sys_run_led_toggle() HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin)

#define key_is_down() 	 (READ_PIN(KEY_IN_GPIO_Port, KEY_IN_Pin) == GPIO_PIN_SET)
#define key_is_release() (READ_PIN(KEY_IN_GPIO_Port, KEY_IN_Pin) == GPIO_PIN_RESET)

void app_init(void);

void app_run(void);

void mavlink_init(void);

void mavlink_process_task(void* arg);

void collect_system_init(void);

void water_test(void);

void param_sensor_start(void);

void param_sensor_update_task_init(void);

void param_sensor_task(void* arg);

uint8_t pipe_cleaning(void);

uint8_t water_collecting(void);

uint8_t measurement_running(void);

void collect_task(void* arg);

void collect_task_init(void);

void usb_task_init(void);
void usb_process_rx_callback(const uint8_t* buf, uint32_t len);

#endif
