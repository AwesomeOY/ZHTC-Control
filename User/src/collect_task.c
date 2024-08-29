#include "app.h"
#include <string.h>

//static uint16_t collect_depth_dm = 0;   // 采水深度，单位分米
static COLLECT_TASK_STATUS  collect_task_status = COLLECT_TASK_STATUS_IDLE; // 当前采集系统运行状态
static COLLECT_TASK_STATUS  old_collect_task_status = COLLECT_TASK_STATUS_IDLE; // 前一次所处状态
static COLLECT_TASK_CMD collect_task_cmd = COLLECT_TASK_CMD_NONE; // 当前执行任务指令
static COLLECT_MODE collect_mode = COLLECT_MODE_NONE;  // 采集控制任务自动模式
static uint32_t _last_manual_time_ms = 0; // 手动模式控制指令最后接收时间
static float _target_depth_mm = 0.0f;
static uint16_t collect_task_index = 0;
extern osEventFlagsId_t collect_event;
extern uint8_t target_bottle_id;
// 留样瓶状态，高位到低位依次为7~0号瓶状态，1-代表满，0-空
extern uint8_t bottle_full_status;

// 阀门开关状态标志；表示0~31 阀门开启状态，1为开启，0为开启中
// pump(b9) | water_out(b8) | water_in(b7) | bottle(b6) | bottle6 ~ bottle1
extern uint16_t vavle_open_status;

/* 当前任务状态 */
inline COLLECT_TASK_STATUS current_collect_task_status(void)
{
	return collect_task_status;
}

inline uint8_t current_collect_task_is_idle(void)
{
	return collect_task_status == COLLECT_TASK_STATUS_IDLE;
}

void collect_task_init(void)
{
	const osThreadAttr_t attributes = {
	  .name = "COLLECT_TASK",
	  .stack_size = 128 * 4,
	  .priority = (osPriority_t)osPriorityLow1,
	};
	osThreadNew(collect_task, NULL, &attributes);
}

/* 水管位置控制 */
uint8_t wait_motor_stop(void)
{
	uint32_t check_flag = MOTOR_ACTION_EVENT_BIT | EXIT_EVENT_BIT | ERROR_EVENT_BIT;
	uint32_t flag = osEventFlagsWait(collect_event, check_flag, osFlagsWaitAny | osFlagsNoClear, 30000U);
	if ((flag <= 0xFFFFFFFA) && (flag & check_flag) > 0) {
		osEventFlagsClear(collect_event, check_flag);
		
		if ((flag & (EXIT_EVENT_BIT | ERROR_EVENT_BIT)) > 0) {
			return 0;
		}
		if (flag & MOTOR_ACTION_EVENT_BIT) {
			return 1;
		}
	}	
	return 0;
}

/* 自动采集任务状态机 */
static void _auto_collect_task(void)
{
	if (collect_mode != COLLECT_MODE_AUTO) {
		return;
	}
	
	switch (collect_task_status)
	{
		case COLLECT_TASK_STATUS_IDLE:	// 等待控制指令
			// 判断取水指令有效
			if (COLLECT_TASK_CMD_START_GET_WATER == collect_task_cmd) {
				old_collect_task_status = COLLECT_TASK_STATUS_IDLE;
				collect_task_status = COLLECT_TASK_STATUS_PUSH_PIPE;
			}				
			break;
		case COLLECT_TASK_STATUS_PAUSE: // 暂停
			break;
		case COLLECT_TASK_STATUS_PUSH_PIPE: // 采水管下降
			// 采水管下降
			// 直到预设深度
			// 进入开启阀门、隔膜泵进入采水润洗状态
			motor_set_position(_target_depth_mm);
			if (wait_motor_stop()) {
				old_collect_task_status = COLLECT_TASK_STATUS_PUSH_PIPE;
				collect_task_status = COLLECT_TASK_STATUS_CLEANING;
			} else {
				collect_task_status = COLLECT_TASK_STATUS_IDLE;
				collect_task_cmd = COLLECT_TASK_CMD_NONE;
			}
			break;
		case COLLECT_TASK_STATUS_CLEANING:  // 润洗
			// 润洗管道三次，每次等待15秒
			// 完成后，关闭排水阀，开启采集水，进入泵吸状态
			if (pipe_cleaning()) {
				old_collect_task_status = COLLECT_TASK_STATUS_CLEANING;
				collect_task_status = COLLECT_TASK_STATUS_PUMP_WATER;
			} else {
				collect_task_status = COLLECT_TASK_STATUS_IDLE;
				collect_task_cmd = COLLECT_TASK_CMD_NONE;
			}					
			break;
		case COLLECT_TASK_STATUS_PUMP_WATER:// 泵吸采水
			// 等待液位有效
			// 进入采水完成状态
			if (water_collecting()) {
				old_collect_task_status = COLLECT_TASK_STATUS_PUMP_WATER;
				collect_task_status = COLLECT_TASK_STATUS_PULL_PIPE;
			} else {
				collect_task_status = COLLECT_TASK_STATUS_IDLE;
				collect_task_cmd = COLLECT_TASK_CMD_NONE;
			}				
			break;
		case COLLECT_TASK_STATUS_PULL_PIPE: // 采水管上升
			motor_set_position(0.0f);
			if (wait_motor_stop()) {
				old_collect_task_status = COLLECT_TASK_STATUS_PULL_PIPE;
				collect_task_status = COLLECT_TASK_STATUS_MEASURE;					
			} else {
				collect_task_status = COLLECT_TASK_STATUS_IDLE;
				collect_task_cmd = COLLECT_TASK_CMD_NONE;
			}
			break;
		case COLLECT_TASK_STATUS_MEASURE:
			if (measurement_running()) {
				++collect_task_index;
				old_collect_task_status = COLLECT_TASK_STATUS_MEASURE;
				collect_task_status = COLLECT_TASK_STATUS_SUCCESS;
			} else {
				collect_task_status = COLLECT_TASK_STATUS_IDLE;
				collect_task_cmd = COLLECT_TASK_CMD_NONE;
			}
			break;
		case COLLECT_TASK_STATUS_CLEAN_BUS: // 排空汇流排
			// 开启排水阀门
			//output_valve_open();
			// 等待一段时间
			// 进入排空支路状态
			break;
		case COLLECT_TASK_STATUS_CLEAN_TREE:// 排空支路
			// 等待一段时间
			// 进入关闭阀门状态，指定排水阀
			break;
		case COLLECT_TASK_STATUS_CLOSE_FAN: // 关闭阀门
			// 关闭指定的阀门
			// 等待阀门完全动作信号
			// 选择进入是放管或者空闲等待
			break;
		case COLLECT_TASK_STATUS_SUCCESS:   // 采水完成
			// 开启测量
			// 等待测量完成
			// 进入空闲状态
			old_collect_task_status = COLLECT_TASK_STATUS_SUCCESS;
			collect_task_status = COLLECT_TASK_STATUS_IDLE;
			collect_task_cmd = COLLECT_TASK_CMD_NONE;
			break;
		case COLLECT_TASK_STATUS_CLEAN_PIPE:// 清洗管路
			break;
		case COLLECT_TASK_STATUS_FAN_CLEAN_PIPE:  // 吹洗管路
			break;
		case COLLECT_TASK_STATUS_MANUAL_PUSH_PIPE:// 水管手动下降
			break;
		case COLLECT_TASK_STATUS_MANUAL_PULL_PIPE:// 水管手动上升
			break;
		case COLLECT_TASK_STATUS_MANUAL_PAUSE_PUSH_PULL:// 水管暂停  
			break;
		default:
			break;
	}
	
	if (collect_task_status == COLLECT_TASK_STATUS_IDLE && old_collect_task_status != collect_task_status) {
		old_collect_task_status = COLLECT_TASK_STATUS_IDLE;
		collect_system_init();
	}
}


/* 手动控制 */
static void _manual_collect_task(void)
{
	if (collect_mode != COLLECT_MODE_MANUAL) {
		return;
	}
	
	uint32_t now = osKernelGetTickCount();
	
	switch (collect_task_status) {
		case COLLECT_TASK_STATUS_MANUAL_PUSH_PIPE:// 水管手动下降
			if (now - _last_manual_time_ms >= 2000U) {
				motor_set_speed(0.0f);
				collect_task_status = COLLECT_TASK_STATUS_MANUAL_PAUSE_PUSH_PULL;
				break;
			}
			motor_set_speed(0.3f);
			break;
		case COLLECT_TASK_STATUS_MANUAL_PULL_PIPE:// 水管手动上升
			if (now - _last_manual_time_ms >= 2000U) {
				motor_set_speed(0.0f);
				collect_task_status = COLLECT_TASK_STATUS_MANUAL_PAUSE_PUSH_PULL;
				break;
			}
			motor_set_speed(-0.3f);
			break;
		case COLLECT_TASK_STATUS_MANUAL_PAUSE_PUSH_PULL:// 水管暂停 
			motor_set_speed(0.0f);
			collect_task_status = COLLECT_TASK_STATUS_IDLE;
			break;
		default:
			break;
	}
}

/* 采水任务 */
void collect_task(void* arg)
{
	while (1)
	{
		switch (collect_mode) {
			case COLLECT_MODE_NONE:
				break;
			case COLLECT_MODE_AUTO:
				_auto_collect_task();
				break;
			case COLLECT_MODE_MANUAL:
				_manual_collect_task();
				break;
		}
		
		osDelay(10);
	}
}


/* 配置系统动作 */
void set_collect_action(COLLECT_TASK_CMD cmd, void* param)
{
	switch (cmd)
	{
		case COLLECT_TASK_CMD_NONE:
			break;
		case COLLECT_TASK_CMD_START_GET_WATER: // 开始取水
			collect_task_cmd = cmd;
			_target_depth_mm = (float)(*((uint32_t*)param));
			break;
		case COLLECT_TASK_CMD_PAUSE_GET_WATER: // 暂停取水
			break;
		case COLLECT_TASK_CMD_RESUME_GET_WATER:// 恢复取水
			break;
		case COLLECT_TASK_CMD_BOTTLE_RESET:    // 瓶状态复位
			break;
		case COLLECT_TASK_CMD_CLEAN_PIPE: // 清洗管路
			break;
		case COLLECT_TASK_CMD_CLOSE_PIPE: // 关闭管路
			break;
		case COLLECT_TASK_CMD_START_PURGE_PIPE: // 吹洗管路
			break;
		case COLLECT_TASK_CMD_STOP_PURGE_PIPE: // 关闭吹洗管路
			break;
		case COLLECT_TASK_CMD_MANUAL_PUSH_PIPE: // 抽水管手动下降
			if (collect_mode == COLLECT_MODE_MANUAL) {
				collect_task_cmd = cmd;
				collect_task_status = COLLECT_TASK_STATUS_MANUAL_PUSH_PIPE;
				_last_manual_time_ms = osKernelGetTickCount();
			}
			break;
		case COLLECT_TASK_CMD_MANUAL_PULL_PIPE: // 抽水管手动上升
			if (collect_mode == COLLECT_MODE_MANUAL) {
				collect_task_cmd = cmd;
				collect_task_status = COLLECT_TASK_STATUS_MANUAL_PULL_PIPE;
				_last_manual_time_ms = osKernelGetTickCount();
			}
			break;			
		case COLLECT_TASK_CMD_MANUAL_PAUSE_PUSH_PULL: // 抽水管暂停上升或者下降
			if (collect_mode == COLLECT_MODE_MANUAL) {
				collect_task_cmd = cmd;
				collect_task_status = COLLECT_TASK_STATUS_MANUAL_PAUSE_PUSH_PULL;
			}
			break;
	} 
}

void collect_protocol_parse(mavlink_data32_t* data32)
{
	collect_data_frame frame;
	memcpy(&frame, data32->data, 32);
	if (frame.msg_id == COLLECT_PROTOCOL_MSG_ID_CMD) {
		uint8_t bottle_id = frame.package.cmd_data.bottle_id;
		COLLECT_MODE mode = (COLLECT_MODE)frame.package.cmd_data.mode;
		ack_data_package ack;
		mavlink_data32_t data32;
		ack.ack = 0;
		ack.msg_id = COLLECT_PROTOCOL_MSG_ID_CMD;
		
		// 目标模式为手动模式，判断自动模式是否处于空闲状态
		// 只有在空闲状态时，才能切入手动模式
		if (mode == COLLECT_MODE_MANUAL) {
			if (collect_mode != COLLECT_MODE_MANUAL && current_collect_task_is_idle()) {
				collect_mode = COLLECT_MODE_MANUAL;
			}
		} else {
			collect_mode = COLLECT_MODE_AUTO;
		}
		
		switch (frame.package.cmd_data.type) {
			case COLLECT_TASK_CMD_START_GET_WATER:
				// 目标瓶已经满，则不能再次开启，应答开启错误
				if (!current_collect_task_is_idle() || (bottle_id != 255 && bottle_is_full((VAVLE_ID_ENUM)bottle_id))) {
					break;
				} else {
					uint32_t mm = 0;
					ack.ack = 1;
					target_bottle_id = bottle_id;
					mm = frame.package.cmd_data.depth_mm;
					set_collect_action(COLLECT_TASK_CMD_START_GET_WATER, (void*)&mm);
				}				
				break;
			case COLLECT_TASK_CMD_STOP:
				ack.ack = 1;
				if (!current_collect_task_is_idle()) {
					osEventFlagsSet(collect_event, EXIT_EVENT_BIT);
				}
				break;
			case COLLECT_TASK_CMD_MANUAL_PUSH_PIPE:      // 抽水管手动下降
				ack.ack = 1;
				set_collect_action(COLLECT_TASK_CMD_MANUAL_PUSH_PIPE, (void*)0);
				break;
			case COLLECT_TASK_CMD_MANUAL_PULL_PIPE:      // 抽水管手动上升
				ack.ack = 1;
				set_collect_action(COLLECT_TASK_CMD_MANUAL_PULL_PIPE, (void*)0);
				break;				
			case COLLECT_TASK_CMD_MANUAL_PAUSE_PUSH_PULL:// 抽水管暂停上升或者下降
				ack.ack = 1;
				set_collect_action(COLLECT_TASK_CMD_MANUAL_PAUSE_PUSH_PULL, (void*)0);
				break;
			default:
				break;
		}
		collect_protocol_ack_data_packed(&data32, &ack);
		mavlink_msg_data32_send_struct((mavlink_channel_t)SERIAL_ID1, (const mavlink_data32_t*)&data32);
	}
}

void collect_protocol_send_heartbeat(void)
{
	heartbeat_data_package hbt;
	mavlink_data32_t data32;
	hbt.collect_mode = (uint8_t)collect_mode;
	hbt.bottle_id = target_bottle_id;
	hbt.collect_status = collect_task_status;
	hbt.collect_current_index = collect_task_index;
	hbt.bottle_status = bottle_full_status;
	hbt.vavle_status = vavle_open_status;
	hbt.target_depth_mm = (uint32_t)_target_depth_mm;
	hbt.current_depth_mm = (uint32_t)get_brt38_angle();
	collect_protocol_heartbeat_data_packed(&data32, &hbt);
	mavlink_msg_data32_send_struct((mavlink_channel_t)SERIAL_ID1, (const mavlink_data32_t*)&data32);
}

void collect_protocol_send_param5(void)
{
	param5_data_package param5;
	mavlink_data32_t data32;
	const ParamSensor5* p5 = get_current_param_sensor5();
	param5.ph_value = p5->ph_value;
	param5.cond_value = p5->cond_value;
	param5.do_value = p5->do_value;
	param5.turb_value = p5->turb_value;
	param5.temp_value = p5->temp_value;
	param5.index = collect_task_index;
	param5.success_flag = p5->success_flag;
	collect_protocol_param5_data_packed(&data32, &param5);
	mavlink_msg_data32_send_struct((mavlink_channel_t)SERIAL_ID1, (const mavlink_data32_t*)&data32);
}

void collect_protocol_send_param4(void)
{
	param4_data_package param4;
	mavlink_data32_t data32;
	const ParamSensor4* p4 = get_current_param_sensor4();
	param4.gao_value = p4->gao_value;
	param4.andan_value = p4->andan_value;
	param4.lin_value = p4->lin_value;
	param4.an_value = p4->an_value;
	param4.index = collect_task_index;
	param4.success_flag = p4->success_flag;
	param4.start_flag = p4->start_flag;
	collect_protocol_param4_data_packed(&data32, &param4);
	mavlink_msg_data32_send_struct((mavlink_channel_t)SERIAL_ID1, (const mavlink_data32_t*)&data32);
}
