#include "RS485_4G.h"
#include "modbus_rtu.h"
#include "serial.h"
#include <string.h>

#define RS485_4G_BUFF_SIZE 128
static uint8_t _rx_buff[RS485_4G_BUFF_SIZE];
static uint8_t _4g_uart_tx_buff[RS485_4G_BUFF_SIZE];
static uint8_t _4g_uart_rx_buff[RS485_4G_BUFF_SIZE];
static uint8_t _rx_len = 0;

static void _4g_receive_cmpt_cb(uint8_t* buf, uint16_t len);

/* 多参数485串口初始化 */
void rs485_4g_init(void)
{
	serial_init(SERIAL_ID4, _4g_uart_rx_buff, sizeof(_4g_uart_rx_buff), SERIAL_DOUBLE_BUF_MODE_DISABLE);
	serial_set_tx_cb(SERIAL_ID4, NULL);
	serial_set_rx_cb(SERIAL_ID4, _4g_receive_cmpt_cb);
}

/* 串口接收数据完成中断，关闭485接收，开启485发送 */
static void _4g_receive_cmpt_cb(uint8_t* buf, uint16_t len)
{
	if (len > 0) {
		_rx_len = len;
		memcpy(_rx_buff, buf, len);
		serial_write(SERIAL_ID4, _rx_buff, len);
	}	
}	
