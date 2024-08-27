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
#include "sys_hw_def.h"
#include "CollectDataProtocol.h"
#include "ParamSensor5.h"
#include "RS485_4G.h"

#define APP_NAME "ZHTC-Control-System"
#define APP_VERSION "V1.3.1"

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
	COLLECT_TASK_CMD_STOP = 13,           // 停止任务
}COLLECT_TASK_CMD;

typedef enum {
	COLLECT_TASK_STATUS_IDLE = 0,           // 等待控制指令
	COLLECT_TASK_STATUS_PAUSE = 1, 			// 暂停
	COLLECT_TASK_STATUS_PUSH_PIPE = 2,      // 采水管下降
	COLLECT_TASK_STATUS_CLEANING = 3,       // 润洗
	COLLECT_TASK_STATUS_PUMP_WATER = 4,     // 泵吸采水
	COLLECT_TASK_STATUS_PULL_PIPE = 5,      // 采水管上升
	COLLECT_TASK_STATUS_MEASURE  = 6,       // 测量中 
	COLLECT_TASK_STATUS_CLEAN_BUS = 7,      // 排空汇流排
	COLLECT_TASK_STATUS_CLEAN_TREE = 8,     // 排空支路
	COLLECT_TASK_STATUS_CLOSE_FAN = 9,      // 关闭阀门
	COLLECT_TASK_STATUS_SUCCESS = 11,       // 采水完成
	COLLECT_TASK_STATUS_CLEAN_PIPE = 12,    // 清洗管路
	COLLECT_TASK_STATUS_FAN_CLEAN_PIPE = 13,  // 吹洗管路  	
	COLLECT_TASK_STATUS_MANUAL_PUSH_PIPE = 14,// 水管手动下降
	COLLECT_TASK_STATUS_MANUAL_PULL_PIPE = 15,// 水管手动上升         
	COLLECT_TASK_STATUS_MANUAL_PAUSE_PUSH_PULL = 16,// 水管暂停
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

typedef enum {
	SWITCH_TYPE_OFF = 0,
	SWITCH_TYPE_ON = 1
}SWITCH_TYPE_ENUM;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

// COLLECT EVENT
#define START_EVENT_BIT          (1U<<0)
#define ERROR_EVENT_BIT          (1U<<1)
#define EXIT_EVENT_BIT           (1U<<2)
#define PARAM_SENSOR_SUCCESS_EVENT_BIT (1U<<3)
#define PARAM_SENSOR_ERROR_EVENT_BIT   (1U<<4)

#define WATER_SW1_ON_EVENT_BIT  (1U<<16)   // 液位开关1有效事件
#define WATER_SW1_OFF_EVENT_BIT (1U<<17)   // 液位开关1无效事件
#define WATER_SW2_ON_EVENT_BIT  (1U<<18)   // 液位开关2有效事件
#define WATER_SW2_OFF_EVENT_BIT (1U<<19)   // 液位开关2无效事件

#define METAL_SW1_ON_EVENT_BIT  (1U<<20)   // 限位开关1有效事件
#define METAL_SW2_ON_EVENT_BIT  (1U<<22)   // 限位开关2有效事件
 
#define MOTOR_ACTION_EVENT_BIT  (1U<<23)   // 电机达到位置事件

// 阀门ID定义 pump(b9) | water_out(b8) | water_in(b7) | bottle(b6) | bottle6 ~ bottle1
typedef enum {
	VAVLE_ID_BOTTLE1 = 0,
	VAVLE_ID_BOTTLE2 = 1,
	VAVLE_ID_BOTTLE3 = 2,
	VAVLE_ID_BOTTLE4 = 3,
	VAVLE_ID_BOTTLE5 = 4,
	VAVLE_ID_BOTTLE6 = 5,
	VAVLE_ID_BOTTLE_ALL = 6,
	VAVLE_ID_WATER_IN   = 7,
	VAVLE_ID_WATER_OUT  = 8,
	VAVLE_ID_PUMP       = 9,
	VAVLE_ID_MAX,
}VAVLE_ID_ENUM;

#define sys_run_led_toggle() HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin)

#define key_is_down() 	 (READ_PIN(KEY_IN_GPIO_Port, KEY_IN_Pin) == GPIO_PIN_SET)
#define key_is_release() (READ_PIN(KEY_IN_GPIO_Port, KEY_IN_Pin) == GPIO_PIN_RESET)

void app_init(void);

void app_run(void);

void mavlink_init(void);

void mavlink_process_task(void* arg);

void collect_system_init(void);

void water_test(void);

void gpio_auto_test(void);

void param_sensor_start(void);

void param_sensor_update_task_init(void);

void param_sensor_task(void* arg);

uint8_t pipe_cleaning(void);

uint8_t water_collecting(void);

uint8_t measurement_running(void);

uint8_t bottle_is_full(VAVLE_ID_ENUM id);

uint8_t vavle_is_open(VAVLE_ID_ENUM id);

void collect_task(void* arg);

void collect_task_init(void);

void gpio_level_output_enable(void);

void gpio_level_output_disable(void);

void usb_task_init(void);
void usb_process_rx_callback(const uint8_t* buf, uint32_t len);

void timer_task_init(void);

void timer_task(void* arg);

COLLECT_TASK_STATUS current_collect_task_status(void);
uint8_t current_collect_task_is_idle(void);

void collect_protocol_send_heartbeat(void);

void collect_protocol_send_param4(void);

void collect_protocol_send_param5(void);

void collect_protocol_parse(mavlink_data32_t* data32);

void course_led_open(void);

void course_led_close(void);

uint8_t metal_sw1_valid(void);

uint8_t metal_sw2_valid(void);

#endif
