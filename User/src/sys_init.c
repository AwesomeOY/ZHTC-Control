#include "app.h"

extern osEventFlagsId_t collect_event;

uint8_t _water_sw1_wait(SWITCH_TYPE_ENUM tp);
uint8_t _water_sw2_wait(SWITCH_TYPE_ENUM tp);
uint8_t bottle_is_full(VAVLE_ID_ENUM id);
uint8_t vavle_is_open(VAVLE_ID_ENUM id);

// 电磁阀完全开/关时间 ms
#define VAVLE_ACTION_TIME_MS 8000U

// 取水和排水最大故障时间
#define WATER_ERROR_TIMEOUT_MS (5U*60U*1000U)

// 四参数电源
gpio_obj four_param_power_gpio = FOUR_PARAM_PW_GOIO_DEF;

// 隔膜泵电源
gpio_obj pump_power_gpio = PUMP_PW_GOIO_DEF;

// 五参数电源
gpio_obj five_param_power_gpio = FIVE_PARAM_PW_GOIO_DEF;

// 四参数进水阀
gpio_obj water_in_valve_power_gpio = WATER_IN_VALVE_PW_GPIO_DEF;

// 排水阀
gpio_obj water_out_valve_power_gpio = WATER_OUT_VALVE_PW_GPIO_DEF;

// 绞盘电机电源开关引脚
gpio_obj motor_power_gpio = MOTOR_PW_GPIO_DEF;

// 航行警示灯
gpio_obj course_led1_power_gpio = LED1_PW_GPIO_DEF;
gpio_obj course_led2_power_gpio = LED2_PW_GPIO_DEF;

// 留样品总开关阀门
gpio_obj bottle_in_valve_power_gpio = REC1_GPIO_DEF;
//REC2_GPIO_DEF

// 液位反馈开关
gpio_obj water_level_sw1_gpio = LEVEL_SW1_IN_GPIO_DEF;
gpio_obj water_level_sw2_gpio = LEVEL_SW2_IN_GPIO_DEF;

// 金属感应器开关输入
gpio_obj metal_sw1_gpio = METAL_SW1_GPIO_DEF;
gpio_obj metal_sw2_gpio = METAL_SW2_GPIO_DEF;

// 留样品进水阀控制IO
gpio_obj bottle1_in_valve_power_gpio = SIGNAL1_OUT_GPIO_DEF;
gpio_obj bottle2_in_valve_power_gpio = SIGNAL2_OUT_GPIO_DEF;
gpio_obj bottle3_in_valve_power_gpio = SIGNAL3_OUT_GPIO_DEF;
gpio_obj bottle4_in_valve_power_gpio = SIGNAL4_OUT_GPIO_DEF;
gpio_obj bottle5_in_valve_power_gpio = SIGNAL5_OUT_GPIO_DEF;
gpio_obj bottle6_in_valve_power_gpio = SIGNAL6_OUT_GPIO_DEF;

uint8_t pre_bottle_id = 0;       // 之前使用的取样瓶号 0 ~ 5
uint8_t current_bottle_id = 0;   // 当前使用的取样瓶号
uint8_t target_bottle_id = 255;  // 配置接下来要使用的留样瓶

// 留样瓶状态，高位到低位依次为7~0号瓶状态，1-代表满，0-空
uint8_t bottle_full_status = 0;

// 阀门开关状态标志；表示0~31 阀门开启状态，1为开启，0为开启中
// pump(b9) | water_out(b8) | water_in(b7) | bottle(b6) | bottle6 ~ bottle1
uint16_t vavle_open_status = 0;

const gpio_obj* bootle_power_gpios[] = {
	&bottle1_in_valve_power_gpio,
	&bottle2_in_valve_power_gpio,
	&bottle3_in_valve_power_gpio,
	&bottle4_in_valve_power_gpio,
	&bottle5_in_valve_power_gpio,
	&bottle6_in_valve_power_gpio
};

const gpio_obj* valve_power_gpios[] = {
	&bottle1_in_valve_power_gpio,
	&bottle2_in_valve_power_gpio,
	&bottle3_in_valve_power_gpio,
	&bottle4_in_valve_power_gpio,
	&bottle5_in_valve_power_gpio,
	&bottle6_in_valve_power_gpio,
	&bottle_in_valve_power_gpio,
	&water_in_valve_power_gpio,
	&water_out_valve_power_gpio,
	&pump_power_gpio
};

/* 电平转换芯片有效 */
void gpio_level_output_enable(void)
{
	WRITE_PIN(GPIOE, GPIO_PIN_7,  GPIO_PIN_SET);
	WRITE_PIN(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	WRITE_PIN(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
	WRITE_PIN(GPIOE, GPIO_PIN_12, GPIO_PIN_SET);
	WRITE_PIN(GPIOD, GPIO_PIN_0,  GPIO_PIN_SET);
}

/* 电平转换芯片无效 */
void gpio_level_output_disable(void)
{
	WRITE_PIN(GPIOE, GPIO_PIN_7,  GPIO_PIN_RESET);
	WRITE_PIN(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
	WRITE_PIN(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
	WRITE_PIN(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET);
	WRITE_PIN(GPIOD, GPIO_PIN_0,  GPIO_PIN_RESET);
}

/* 采集系统初始化; 关闭所有阀门 */
void collect_system_init(void)
{
	uint8_t i = 0;
	for (i = 0; i<sizeof(bootle_power_gpios)/sizeof(bootle_power_gpios[0]); ++i) {
		gpio_output_invalid(bootle_power_gpios[i]);
	}
	gpio_output_invalid(&water_in_valve_power_gpio);
	gpio_output_invalid(&water_out_valve_power_gpio);	
	gpio_output_invalid(&bottle_in_valve_power_gpio);
	gpio_output_invalid(&pump_power_gpio);
	gpio_output_invalid(&four_param_power_gpio);
	gpio_output_invalid(&five_param_power_gpio);
	
	vavle_open_status = 0;
}

/* 留样瓶阀门控制,会存在开关时间的阻塞 */
uint8_t valve_control(VAVLE_ID_ENUM id, uint8_t open, uint8_t waitEn)
{	
	// ID无效
	if (id >= VAVLE_ID_MAX) {
		return 0;
	}
	
	// 留样瓶中的样本已经是满状态，则不能再次开启阀门
	if (open && bottle_is_full(id)) {
		return 0;
	}
	
	// 留样瓶已经是开启状态
	if (open && vavle_is_open(id)) {
		return 0;
	}
	
	// 留样瓶已经是关闭状态
	if (!open && (vavle_is_open(id) == 0)) {
		return 0;
	}
	
	if (open) {
		gpio_output_valid(valve_power_gpios[id]);
		vavle_open_status |= 1 << id;
	} else {
		gpio_output_invalid(valve_power_gpios[id]);
		vavle_open_status &= ~(1 << id);
	}
	
	// 其他阀门需要控制时间
	if ((id != VAVLE_ID_PUMP) && waitEn) {
		osDelay(VAVLE_ACTION_TIME_MS);
	}
	
	return 1;	
}

/* 判断留样瓶是否已经满 */
inline uint8_t bottle_is_full(VAVLE_ID_ENUM id)
{
	if (id >= VAVLE_ID_BOTTLE_ALL) {
		return 0;
	}
	return (bottle_full_status & (1<<id)) > 0;
}

/* 判断阀门是否开启 */
inline uint8_t vavle_is_open(VAVLE_ID_ENUM id)
{
	if (id >= VAVLE_ID_MAX) {
		return 1;
	}
	return (vavle_open_status & (1U<<id)) > 0;
}

/* 清洗管路
** 1、首先确定水管放置到了目标水深
** 2、关闭所有的留样阀、排水阀；开启多参池阀、留样总阀
** 3、开启隔膜泵取水
** 4、等待多参液位开关有效，则开启排水阀，等待排水完成(20s)
** 5、重复以上步骤3次
** 6、最后关闭排水阀，等待取水
** 注意: 液位开关动作需做时间检查，如果超时，则报警，返回0
*/
uint8_t pipe_cleaning(void)
{
	uint8_t i = 0;
	
	// 关闭留样阀
	for (i = 0; i < sizeof(bootle_power_gpios)/sizeof(&bootle_power_gpios[0]); ++i) {
		valve_control((VAVLE_ID_ENUM)i, 0, 0);
	}
	osDelay(VAVLE_ACTION_TIME_MS);
	
	// 开启多参池阀，留样总阀
	valve_control(VAVLE_ID_BOTTLE_ALL, 1, 0);
	valve_control(VAVLE_ID_WATER_IN, 1, 1);
	
	uint8_t loop = 0;
	for (loop = 0; loop < 3; ++loop) {
		
		// 关闭排水阀
		valve_control(VAVLE_ID_WATER_OUT, 0, 1);
		
		// 开启隔膜泵取水
		valve_control(VAVLE_ID_PUMP, 1, 0);
		
		// 等待液位开关有效
		if (!_water_sw1_wait(SWITCH_TYPE_ON)) {
			return 0;
		}
		
		// 关闭隔膜泵
		valve_control(VAVLE_ID_PUMP, 0, 0);
		
		// 开始润洗排水
		valve_control(VAVLE_ID_WATER_OUT, 1, 1);
		
		// 等待液位开关为低液位状态
		if (!_water_sw1_wait(SWITCH_TYPE_OFF)) {
			return 0;
		}
		osDelay(5000U);
	}
	
	// 关闭润洗阀
	valve_control(VAVLE_ID_WATER_OUT, 0, 1);
	
	return 1;
}

/* 开始采集水
*  1、开启多参数进水阀
*  2、开启留样瓶阀门
*  3、开启隔膜泵
*  3、等待液位有效
*  4、关闭隔膜泵
*/

uint8_t water_collecting(void)
{
	// 开启多参数进水阀、留样瓶总阀、对应的留样瓶阀
	valve_control(VAVLE_ID_WATER_OUT, 0, 1);
		
	valve_control(VAVLE_ID_WATER_IN, 1, 0);
	valve_control(VAVLE_ID_BOTTLE_ALL, 1, 0);
	if (target_bottle_id == 255 || valve_control((VAVLE_ID_ENUM)target_bottle_id, 1, 1)) {
		current_bottle_id = target_bottle_id;
	} else {
		return 0;
	}
	
	// 开启隔膜泵
	valve_control(VAVLE_ID_PUMP, 1, 0);
	
	// 等待液位开关有效
	if (!_water_sw1_wait(SWITCH_TYPE_ON)) {
		return 0;
	}
	
	// 关闭留样瓶阀，并标志留样品满
	if (target_bottle_id != 255) {
		valve_control((VAVLE_ID_ENUM)target_bottle_id, 0, 1);
	}	
	
	// 关闭隔膜泵
	valve_control(VAVLE_ID_PUMP, 0, 0);
	return 1;
}

///* 放管
//* 
//*/
//uint8_t motor_push(void)
//{
//	return 1;
//}

///* 收管
//*
//*/
//uint8_t motor_pull(void)
//{
//	return 1;
//}

/* 进入仪器采样计算阶段
*  1、延时等待60s
*  2、五参数电源通电
*  3、四参数电源通电
*  4、发送开始检测指令
*/
uint8_t measurement_running(void)
{
	uint32_t flag = 0;
	uint32_t check_flag = 0;
	// 开启五、四参数电源
	gpio_output_valid(&four_param_power_gpio);
	gpio_output_valid(&five_param_power_gpio);
	
	// 发送开始检测指令，进入仪器测量状态
	param_sensor_start();
	
	// 读取仪器状态
	check_flag = PARAM_SENSOR_SUCCESS_EVENT_BIT | PARAM_SENSOR_ERROR_EVENT_BIT | EXIT_EVENT_BIT | ERROR_EVENT_BIT;
	flag = osEventFlagsWait(collect_event, check_flag, osFlagsWaitAny, 80000u);
	if ((flag & check_flag) > 0) {
		// 读取仪器测量数据，整个流程完成
	}
	
	// 关闭所有外设电源：电磁阀、四五参数电源、隔膜泵
	collect_system_init();
	
	return 1;
}


/*********************************************************/
/*** 测试代码 ***/
void water_test(void)
{
//	float ang = 0.0f;
	// 判断电机角度是否为0，不为0，则上升
//	while (1) {
//		if (brt38_is_exist()) {
//			ang = get_brt38_angle();
//			if (ang <= 2.0f || ang >= 7000.0f) {
//				motor_set_speed(0.0f);
//				brt38_set_reset();
//				while (0 == brt38_get_reset_status()); // 等待复位
//				break;
//			} else {
//				motor_set_speed(-0.5f);
//			}
//		}
//	}
//	
//	osDelay(1000);
//	// 下放长度
//	while (1) {
//		ang = get_brt38_angle();
//		if (ang >= 150.0f || fabsf(ang - 150.0f) <= 2.0f) {
//			motor_set_speed(0.0f);
//			break;
//		} else {
//			motor_set_speed(0.5f);
//		}
//	}
	
	// 开启进水阀门
	gpio_output_valid(&water_in_valve_power_gpio);
	osDelay(8000);
	
	// 开启水泵
	gpio_output_valid(&pump_power_gpio);
	
	// 判断水位开关，到达，则关闭进水阀
	_water_sw1_wait(SWITCH_TYPE_OFF);
	
	// 关闭水泵
	gpio_output_invalid(&water_in_valve_power_gpio);
	gpio_output_invalid(&pump_power_gpio);
}

void gpio_auto_test(void)
{
	uint8_t i = 0;
	gpio_output_valid(&four_param_power_gpio);
	osDelay(2000);
	
	gpio_output_valid(&five_param_power_gpio);
	osDelay(2000);
	
	gpio_output_valid(&water_out_valve_power_gpio);
	osDelay(2000);
	
	gpio_output_valid(&water_in_valve_power_gpio);
	osDelay(2000);
	
	gpio_output_valid(&pump_power_gpio);
	osDelay(2000);
	
	gpio_output_valid(&motor_power_gpio);
	osDelay(2000);
	
	gpio_output_valid(&course_led1_power_gpio);
	osDelay(2000);
	
	gpio_output_valid(&course_led2_power_gpio);
	osDelay(2000);
	
	gpio_output_valid(&bottle_in_valve_power_gpio);
	osDelay(2000);
	
	for (i = 0; i<sizeof(bootle_power_gpios)/sizeof(bootle_power_gpios[0]); ++i) {
		gpio_output_valid(bootle_power_gpios[i]);
		osDelay(2000);
	}
	
	osDelay(5000);
	
	gpio_output_invalid(&four_param_power_gpio);
	osDelay(2000);
	
	gpio_output_invalid(&five_param_power_gpio);
	osDelay(2000);
	
	gpio_output_invalid(&water_out_valve_power_gpio);
	osDelay(2000);
	
	gpio_output_invalid(&water_in_valve_power_gpio);
	osDelay(2000);
	
	gpio_output_invalid(&pump_power_gpio);
	osDelay(2000);
	
	gpio_output_invalid(&motor_power_gpio);
	osDelay(2000);
	
	gpio_output_invalid(&course_led1_power_gpio);
	osDelay(2000);
	
	gpio_output_invalid(&course_led2_power_gpio);
	osDelay(2000);
	
	gpio_output_invalid(&bottle_in_valve_power_gpio);
	osDelay(2000);
	
	for (i = 0; i<sizeof(bootle_power_gpios)/sizeof(bootle_power_gpios[0]); ++i) {
		gpio_output_invalid(bootle_power_gpios[i]);
		osDelay(2000);
	}
}

/* 等待液位开关1信号动作 */
uint8_t _water_sw1_wait(SWITCH_TYPE_ENUM tp)
{
	if (tp == SWITCH_TYPE_ON) {
		uint32_t check_flag = WATER_SW1_ON_EVENT_BIT | EXIT_EVENT_BIT | ERROR_EVENT_BIT;
		uint32_t flag = 0;
		osEventFlagsClear(collect_event, WATER_SW1_ON_EVENT_BIT);	
		flag = osEventFlagsWait(collect_event, check_flag, osFlagsWaitAny | osFlagsNoClear, WATER_ERROR_TIMEOUT_MS);
		if ((flag & check_flag) > 0) {
			osEventFlagsClear(collect_event, check_flag);
			
			if ((flag & (EXIT_EVENT_BIT | ERROR_EVENT_BIT)) > 0) {
				return 0;
			}
			if (flag & WATER_SW1_ON_EVENT_BIT) {
				if (gpio_input_valid(&water_level_sw1_gpio)) {
					osDelay(20); // 等待电平信号稳定
					if (gpio_input_valid(&water_level_sw1_gpio)) {
						return 1;
					}
				}
			}
		}
	}
	else if (tp == SWITCH_TYPE_OFF) {
		uint32_t check_flag = WATER_SW1_OFF_EVENT_BIT | EXIT_EVENT_BIT | ERROR_EVENT_BIT;
		uint32_t flag = 0;
		osEventFlagsClear(collect_event, WATER_SW1_OFF_EVENT_BIT);
		flag = osEventFlagsWait(collect_event, check_flag, osFlagsWaitAny | osFlagsNoClear, WATER_ERROR_TIMEOUT_MS);
		if ((flag & check_flag) > 0) {
			osEventFlagsClear(collect_event, check_flag);
			
			if ((flag & (EXIT_EVENT_BIT | ERROR_EVENT_BIT)) > 0) {
				return 0;
			}
			if (flag & WATER_SW1_OFF_EVENT_BIT) {
				if (gpio_input_valid(&water_level_sw1_gpio) == 0) {
					osDelay(20); // 等待电平信号稳定
					if (gpio_input_valid(&water_level_sw1_gpio) == 0) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

/* 等待液位开关2信号有效 */
uint8_t _water_sw2_wait(SWITCH_TYPE_ENUM tp)
{
	if (tp == SWITCH_TYPE_ON) {
		uint32_t check_flag = WATER_SW2_ON_EVENT_BIT | EXIT_EVENT_BIT | ERROR_EVENT_BIT;
		uint32_t flag = 0;
		osEventFlagsClear(collect_event, WATER_SW2_ON_EVENT_BIT);
		flag = osEventFlagsWait(collect_event, check_flag, osFlagsWaitAny | osFlagsNoClear, WATER_ERROR_TIMEOUT_MS);
		if ((flag & check_flag) > 0) {
			osEventFlagsClear(collect_event, check_flag);
			
			if ((flag & (EXIT_EVENT_BIT | ERROR_EVENT_BIT)) > 0) {
				return 0;
			}
			if (flag & WATER_SW2_ON_EVENT_BIT) {
				if (gpio_input_valid(&water_level_sw2_gpio)) {
					osDelay(20); // 等待电平信号稳定
					if (gpio_input_valid(&water_level_sw2_gpio)) {
						return 1;
					}
				}
			}
		}
	}
	else if (tp == SWITCH_TYPE_OFF) {
		uint32_t check_flag = WATER_SW2_OFF_EVENT_BIT | EXIT_EVENT_BIT | ERROR_EVENT_BIT;
		uint32_t flag = 0;
		osEventFlagsClear(collect_event, WATER_SW2_OFF_EVENT_BIT);
		flag = osEventFlagsWait(collect_event, check_flag, osFlagsWaitAny | osFlagsNoClear, WATER_ERROR_TIMEOUT_MS);
		if ((flag & check_flag) > 0) {
			osEventFlagsClear(collect_event, check_flag);
			
			if ((flag & (EXIT_EVENT_BIT | ERROR_EVENT_BIT)) > 0) {
				return 0;
			}
			if (flag & WATER_SW2_OFF_EVENT_BIT) {
				if (gpio_input_valid(&water_level_sw2_gpio) == 0) {
					osDelay(20); // 等待电平信号稳定
					if (gpio_input_valid(&water_level_sw2_gpio) == 0) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

