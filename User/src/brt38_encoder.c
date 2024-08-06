#include "brt38_encoder.h"
#include "modbus_rtu.h"
#include "serial.h"
#include "cmsis_os2.h"
#include "main.h"
#include <string.h>

#define brt38_rs485_tx_enable() 
#define brt38_rs485_rx_enable() 

#define BRT38_MODBUS_ADDR 0x01

#define BRT38_UART_MAX_BUFF_SIZE 64

__IO uint8_t brt38_rx_len = 0; // 串口接收数据的总长度
uint8_t brt38_rx_buff[BRT38_UART_MAX_BUFF_SIZE];
uint8_t brt38_uart_rx_buff[BRT38_UART_MAX_BUFF_SIZE];
uint8_t brt38_uart_tx_buff[BRT38_UART_MAX_BUFF_SIZE];

float brt38_angle = 0.0f; // 编码器当前多圈角度
uint8_t brt38_reset_success = 0;
uint8_t brt38_need_reset = 0; // 设置复位标志
uint8_t brt38_exist = 0; // 编码器存在标志

static void brt38_process_task(void* arg);
static void brt38_receive_cmpt_cb(uint8_t* buf, uint16_t len);
static void brt38_send_cmpt_cb(void* arg);

osThreadId_t brt38TaskHandle;
const osThreadAttr_t brt38Task_attributes = {
  .name = "Brt38_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t)osPriorityAboveNormal7,
};

void brt38_encoder_init(void)
{
	brt38_rs485_tx_enable();
	serial_init(SERIAL_ID6, brt38_uart_rx_buff, BRT38_UART_MAX_BUFF_SIZE, SERIAL_DOUBLE_BUF_MODE_DISABLE);
	serial_set_tx_cb(SERIAL_ID6, brt38_send_cmpt_cb);
	serial_set_rx_cb(SERIAL_ID6, brt38_receive_cmpt_cb);
	
	brt38TaskHandle = osThreadNew(brt38_process_task, NULL, &brt38Task_attributes);
}

/* 返回当前编码角度 */
inline float get_brt38_angle(void)
{
	return brt38_angle;
}

/* 编码器置零，复位 */
uint8_t brt38_hw_reset(void)
{
	uint8_t sendSize = modbus_rtu_write_single_reg_pack(brt38_uart_tx_buff, BRT38_MODBUS_ADDR, 0x08, 0x01);
	
	brt38_rx_len = 0;
	brt38_reset_success = 0;
	brt38_rs485_tx_enable();
	if (serial_write(SERIAL_ID6, brt38_uart_tx_buff, sendSize) > 0) {
		if (serial_can_read(SERIAL_ID6, 5000) && brt38_rx_len > 0) {
			if (modbus_rtu_check_crc(brt38_rx_buff, brt38_rx_len)) {
				brt38_reset_success = 1;
				osDelay(1000);
				if (brt38_set_dir(0)) {
					return 1;
				}
			}
		}
	}
		
	return 0;
}

/* 设置方向 */
uint8_t brt38_set_dir(uint8_t cw)
{
	cw = cw > 0 ? 0 : 1;
	uint8_t sendSize = modbus_rtu_write_single_reg_pack(brt38_uart_tx_buff, BRT38_MODBUS_ADDR, 0x09, cw);
	
	brt38_rs485_tx_enable();
	if (serial_write(SERIAL_ID6, brt38_uart_tx_buff, sendSize) > 0) {
		if (serial_can_read(SERIAL_ID6, 5000) && brt38_rx_len > 0) {
			if (modbus_rtu_check_crc(brt38_rx_buff, brt38_rx_len)) {
				return 1;
			}
		}
	}
		
	return 0;
}

/* 获取编码器值，寄存器地址为0x00 ~ 0x01 */
uint8_t brt38_get_encoder_value(uint32_t* value)
{
	uint8_t sendSize = modbus_rtu_read_reg_pack(brt38_uart_tx_buff, BRT38_MODBUS_ADDR, 0x00, 0x02);
	
	brt38_rx_len = 0;
	brt38_rs485_tx_enable();
	if (serial_write(SERIAL_ID6, brt38_uart_tx_buff, sendSize) > 0) {
		if (serial_can_read(SERIAL_ID6, 5000) && brt38_rx_len > 0) {
			if (modbus_rtu_check_crc(brt38_rx_buff, brt38_rx_len)) {
				uint16_t v[2] = {0, 0};
				if (modbus_rtu_read_reg(brt38_rx_buff, brt38_rx_len, v)) {
					*value = ((uint32_t)v[0] << 16) | v[1];
					brt38_exist = 1;
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
		if (brt38_need_reset) {
			if (brt38_hw_reset()) {
				brt38_need_reset = 0;
			}
		}
		
		uint32_t encoder_value;
		if (brt38_get_encoder_value(&encoder_value)) {
			brt38_angle = encoder_value * 360 / 1024.0f;
		}
		osDelay(20);
	}
}


/////////////////////////////////////////////////////////////////

/* 串口接收数据完成中断，关闭485接收，开启485发送 */
static void brt38_receive_cmpt_cb(uint8_t* buf, uint16_t len)
{
	brt38_rs485_tx_enable(); // 关闭接收
	if (len > 0) {
		brt38_rx_len = len;
		memcpy(brt38_rx_buff, buf, len);
	}	
}

/* 串口发送完成中断中调用，关闭485发送，开启485接收 */
static void brt38_send_cmpt_cb(void* arg)
{
	brt38_rs485_rx_enable(); // 开启接收
}

inline uint8_t brt38_get_reset_status(void)
{
	return brt38_reset_success;
}

/* 设置brt38复位 */
inline void brt38_set_reset(void)
{
	brt38_reset_success = 0;
	brt38_need_reset = 1;
}

/* brt38存在 */
inline uint8_t brt38_is_exist(void)
{
	return brt38_exist;
}