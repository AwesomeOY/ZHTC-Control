#include "brt38_encoder.h"
#include "modbus_rtu.h"
#include "serial.h"
#include "cmsis_os2.h"
#include "main.h"
#include <string.h>

#define brt38_rs485_tx_enable() HAL_GPIO_WritePin(RS485_EN1_GPIO_Port, RS485_EN1_Pin, GPIO_PIN_SET)
#define brt38_rs485_rx_enable() HAL_GPIO_WritePin(RS485_EN1_GPIO_Port, RS485_EN1_Pin, GPIO_PIN_RESET)

#define BRT38_MODBUS_ADDR 0x01

#define BRT38_UART_MAX_BUFF_SIZE 64

__IO uint8_t brt38_rx_len = 0; // 串口接收数据的总长度
uint8_t brt38_rx_buff[BRT38_UART_MAX_BUFF_SIZE];
uint8_t brt38_uart_rx_buff[BRT38_UART_MAX_BUFF_SIZE];
uint8_t brt38_uart_tx_buff[BRT38_UART_MAX_BUFF_SIZE];

float brt38_angle = 0.0f; // 编码器当前多圈角度

static void brt38_process_task(void* arg);
static void brt38_receive_cmpt_cb(void* arg);
static void brt38_send_cmpt_cb(void* arg);

osThreadId_t brt38TaskHandle;
const osThreadAttr_t brt38Task_attributes = {
  .name = "Brt38_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t)osPriorityNormal1,
};

void brt38_encoder_init(void)
{
	brt38_rs485_tx_enable();
	serial_init(SERIAL_ID1, brt38_uart_rx_buff, BRT38_UART_MAX_BUFF_SIZE);
	serial_set_tx_cb(SERIAL_ID1, brt38_send_cmpt_cb);
	serial_set_rx_cb(SERIAL_ID1, brt38_receive_cmpt_cb);
	
	brt38TaskHandle = osThreadNew(brt38_process_task, NULL, &brt38Task_attributes);
}

/* 返回当前编码角度 */
float get_brt38_angle(void)
{
	return brt38_angle;
}

/* 编码器置零，复位 */
void brt38_hw_reset(void)
{
	
}

/* 获取编码器值，寄存器地址为0x00 ~ 0x01 */
uint8_t brt38_get_encoder_value(uint32_t* value)
{
	uint8_t sendSize = modbus_rtu_read_reg_pack(brt38_uart_tx_buff, BRT38_MODBUS_ADDR, 0x00, 0x02);
	
	brt38_rx_len = 0;
	brt38_rs485_tx_enable();
	if (serial_write(SERIAL_ID1, brt38_uart_tx_buff, sendSize) > 0) {
		if (serial_can_read(SERIAL_ID1, 5000) && brt38_rx_len > 0) {
			if (modbus_rtu_check_crc(brt38_rx_buff, brt38_rx_len)) {
				uint16_t v[2] = {0, 0};
				if (modbus_rtu_read_reg(brt38_rx_buff, brt38_rx_len, v)) {
					*value = ((uint32_t)v[0] << 16) | v[1];
					return 1;
				}
			}
		}
	}
		
	return 0;
}

/* 操作BRT38编码器任务 */
static void brt38_process_task(void* arg)
{
	uint32_t last_time_ms = osKernelGetTickCount();
	
	while (1) {
		uint32_t now = osKernelGetTickCount();
		if (now - last_time_ms >= 1000) {
			uint32_t encoder_value;
			if (brt38_get_encoder_value(&encoder_value)) {
				brt38_angle = encoder_value * 360 / 1024.0f;
			}
		}
	}
}



/////////////////////////////////////////////////////////////////

/* 串口接收数据完成中断，关闭485接收，开启485发送 */
static void brt38_receive_cmpt_cb(void* arg)
{
	brt38_rs485_tx_enable(); // 关闭接收
	serial_obj* pso = (serial_obj*)arg;
	if (pso->rx_len > 0) {
		brt38_rx_len = pso->rx_len;
		memcpy(brt38_rx_buff, pso->p_rx_buff, pso->rx_len);
	}	
}

/* 串口发送完成中断中调用，关闭485发送，开启485接收 */
static void brt38_send_cmpt_cb(void* arg)
{
	brt38_rs485_rx_enable(); // 开启接收
}

