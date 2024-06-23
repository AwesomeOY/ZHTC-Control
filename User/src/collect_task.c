#include "app.h"

static uint16_t collect_depth_dm = 0;   // 采水深度，单位分米
static COLLECT_TASK_STATUS  collect_task_status = COLLECT_TASK_STATUS_IDLE; // 当前采集系统运行状态
static COLLECT_TASK_STATUS  old_collect_task_status = COLLECT_TASK_STATUS_IDLE; // 前一次所处状态
static COLLECT_TASK_CMD collect_task_cmd = COLLECT_TASK_CMD_NONE; // 当前执行任务指令

void collect_task(void* arg)
{
	while (1)
	{
		switch (collect_task_status)
		{
			case COLLECT_TASK_STATUS_IDLE:	// 等待控制指令
				// 判断取水指令有效
				break;
			case COLLECT_TASK_STATUS_PAUSE: // 暂停
				break;
			case COLLECT_TASK_STATUS_PUSH_PIPE: // 采水管下降
				// 采水管下降
			    // 直到预设深度
			    // 进入开启阀门、隔膜泵进入采水润洗状态
				break;
			case COLLECT_TASK_STATUS_CLEANING:  // 润洗
				// 润洗管道三次，每次等待15秒
			    // 完成后，关闭排水阀，开启采集水，进入泵吸状态
				break;
			case COLLECT_TASK_STATUS_PUMP_WATER:// 泵吸采水
				// 等待液位有效
			    // 进入采水完成状态
				break;
			case COLLECT_TASK_STATUS_PULL_PIPE: // 采水管上升
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
			break;
		case COLLECT_TASK_CMD_MANUAL_PULL_PIPE: // 抽水管手动上升
			break;			
		case COLLECT_TASK_CMD_MANUAL_PAUSE_PUSH_PULL: // 抽水管暂停上升或者下降
			break;
	} 
}
