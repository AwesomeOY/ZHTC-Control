#include "app.h"

// 液位反馈开关
extern gpio_obj water_level_sw1_gpio;
extern gpio_obj water_level_sw2_gpio;

// 金属感应器开关输入
extern gpio_obj metal_sw1_gpio;
extern gpio_obj metal_sw2_gpio;

extern osEventFlagsId_t collect_event;

typedef struct {
	uint8_t changed;      // 引脚电平被改变
	uint8_t old_status;   // 上一次判断有效状态
	uint32_t last_time_ms;
	const gpio_obj* p_gpio;
}gpio_input_class;

static float _last_pos = 0.0f;
static uint8_t _last_update_pos_count = 0;

static void _gpio_inputs_init(void);
static void _gpio_input_update(gpio_input_class* p_input);
static void _gpio_callback(const gpio_input_class* p_input, uint8_t valid);
static int8_t _position_control(float pos);

osSemaphoreId_t timer_task_sem;
gpio_input_class water_sw1_input = { 0, 0, 0, &water_level_sw1_gpio};
gpio_input_class water_sw2_input = { 0, 0, 0, &water_level_sw2_gpio};
gpio_input_class metal_sw1_input = { 0, 0, 0, &metal_sw1_gpio};
gpio_input_class metal_sw2_input = { 0, 0, 0, &metal_sw2_gpio};

gpio_input_class* gpio_inputs[] = {
	&water_sw1_input,
	&water_sw2_input,
	&metal_sw1_input,
	&metal_sw2_input
};

const osThreadAttr_t timerTask_attributes = {
  .name = "TIMER_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t)osPriorityHigh,
};

void timer_task_init(void)
{
	osSemaphoreAttr_t attr = { "TIMER_SEM",  0, NULL };
	_gpio_inputs_init();
	timer_task_sem = osSemaphoreNew(1, 0, &attr);
	osThreadNew(timer_task, NULL, &timerTask_attributes);
}

void timer_task(void* arg)
{
	while (1) {
		if (osOK == osSemaphoreAcquire(timer_task_sem, 0xFFFFFFFF)) {
			uint8_t i = 0;
			for (i = 0; i < sizeof(gpio_inputs)/sizeof(gpio_inputs[0]); ++i) {
				_gpio_input_update(gpio_inputs[i]);
			}

			if (motor_need_control()) {
				_position_control(motor_get_target_pos());
			}			
		}
	}
}

/* 水管位置控制 */
static int8_t _position_control(float pos)
{
	float cpos = get_brt38_angle();
	
	// 目标深度为0，并且超出范围，则强制停止，再进行编码器复位
	if (pos <= 0.001f && cpos >= 7000.0f) {
		motor_set_speed(0.0f);
		brt38_set_reset();
		_last_update_pos_count = 0;
		motor_power_off();
		osDelay(1000);
		motor_power_on();
		osDelay(2000);
		return 1;
	}
	
	// 判断角度是否变化
	if (fabsf(_last_pos - cpos) <= 0.01f) {
		++_last_update_pos_count;
	} else {
		_last_update_pos_count = 0;
	}
	_last_pos = cpos;
	
	// 误差为两个mm，则停下
	if (fabsf(pos - cpos) <= 2.0f || _last_update_pos_count >= 50) {
		motor_set_speed(0.0f);
		_last_update_pos_count = 0;
		
		// 出现阻塞情况则重启电机电源
		if (_last_update_pos_count >= 50) {
			motor_power_off();
			osDelay(1000);
			motor_power_on();
			osDelay(2000);
		}
		
		// 如果目标值为0，则选择复位
		if (pos <= 0.001f) {
			brt38_set_reset();
			motor_power_off();
			osDelay(1000);
			motor_power_on();
			osDelay(2000);
		}
		osEventFlagsClear(collect_event, MOTOR_ACTION_EVENT_BIT);
		osEventFlagsSet (collect_event, MOTOR_ACTION_EVENT_BIT);
		return 1;
	} else {
		if (pos - cpos > 2.0f) {
			motor_set_speed(0.5f);
		} if (pos - cpos < -2.0f) {
			motor_set_speed(-0.5f);
		}
	}
	
	return 0;
}

static void _gpio_inputs_init(void)
{
	uint8_t i = 0;
	for (i = 0; i < sizeof(gpio_inputs)/sizeof(gpio_inputs[0]); ++i) {
		gpio_inputs[0]->old_status = gpio_input_valid(gpio_inputs[0]->p_gpio);
	}	
}


/* 判断输入IO口电平是否有效， 选择调用回调函数 */
static void _gpio_input_update(gpio_input_class* p_input)
{
	uint8_t status = gpio_input_valid(p_input->p_gpio);
	uint32_t now = osKernelGetTickCount();
	
	if (p_input->old_status != status) {
		p_input->changed = 1;
		p_input->old_status = status;
		return;
	}
	
	if (p_input->changed || (now - p_input->last_time_ms) >= 20U) {
		p_input->changed = 0;
		p_input->last_time_ms = now;
		_gpio_callback(p_input, status);
	}
}

/* GPIO中断回调响应
** 液位开关高电平有效
** 限位开关低电平有效
*/
void _gpio_callback(const gpio_input_class* p_input, uint8_t valid)
{
	// PB0，液位开关1
	if (&water_level_sw1_gpio == p_input->p_gpio) {
		if (valid) {
			osEventFlagsClear(collect_event, WATER_SW1_OFF_EVENT_BIT);
			osEventFlagsSet(collect_event, WATER_SW1_ON_EVENT_BIT);
		} else {
			osEventFlagsClear(collect_event, WATER_SW1_ON_EVENT_BIT);
			osEventFlagsSet(collect_event, WATER_SW1_OFF_EVENT_BIT);
		}
	}
	
	// PC5，液位开关2
	else if (&water_level_sw2_gpio == p_input->p_gpio) {
		if (valid) {
			osEventFlagsClear(collect_event, WATER_SW2_OFF_EVENT_BIT);
			osEventFlagsSet(collect_event, WATER_SW2_ON_EVENT_BIT);
		} else {
			osEventFlagsClear(collect_event, WATER_SW2_ON_EVENT_BIT);
			osEventFlagsSet(collect_event, WATER_SW2_OFF_EVENT_BIT);
		}
	}
	
	// PC4，限位开关1，用于复位电机编码器，强制停止控制
	else if (&metal_sw1_gpio == p_input->p_gpio) {
		if (valid) {
			if (motor_get_target_pos() <= 0.001f) {
				motor_stop();      // 关闭电机控制
				if (get_brt38_angle() >= 0.01f) {
					brt38_set_reset(); // 需要复位编码器，复位零点
				}				
			}
		}
	}
	
	// PA7，限位开关2，用于确定水管已经被拉到最长，强制停止控制
	else if (&metal_sw2_gpio == p_input->p_gpio) {
		if (valid) {
			motor_stop();
		}
	}
	
	else {
	
	}
}
