#include "serial.h"

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

UART_HandleTypeDef* uart_handles[SERIAL_ID_MAX] = { &huart1, &huart2, &huart3, &huart4, &huart5, &huart6 };
serial_obj serial_objs[SERIAL_ID_MAX];

void serial_init(SERIAL_ID id, uint8_t* rbuf, uint16_t rlen)
{
	char name[] = {"SER_TX_SEM0"};
	osSemaphoreAttr_t attr = { NULL,  0, NULL };
	
	serial_objs[(uint8_t)id].p_rx_buff = rbuf;
	serial_objs[(uint8_t)id].rx_buff_size = rlen;
	
	if (serial_objs[(uint8_t)id].init == 0) {
		name[4] = 'T';
		name[10] = '0' + (id%16);
		attr.name = name;
		serial_objs[(uint8_t)id].tx_sem = osSemaphoreNew(1, 0, &attr);
		
		name[4] = 'R';
		name[10] = '0' + (id%16);
		attr.name = name;
		serial_objs[(uint8_t)id].rx_sem = osSemaphoreNew(1, 0, &attr);
		
		serial_objs[(uint8_t)id].init = 1;
		
		serial_objs[(uint8_t)id].huart = uart_handles[(uint8_t)id];
		
		serial_objs[(uint8_t)id].cand_send = 1;
		
		HAL_UARTEx_ReceiveToIdle_DMA(serial_objs[(uint8_t)id].huart, serial_objs[(uint8_t)id].p_rx_buff, serial_objs[(uint8_t)id].rx_buff_size);
	}	
}

/* 指定缓冲区将串口数据发送出去 */
uint16_t serial_write(SERIAL_ID id, uint8_t* pbuf, uint16_t len)
{
	HAL_UART_Transmit_DMA(uart_handles[(uint8_t)id], pbuf, len);
	return len;
}

uint8_t serial_can_write(SERIAL_ID id, uint32_t timeout)
{
	return serial_objs[(uint8_t)id].cand_send;
}

uint8_t serial_can_read(SERIAL_ID id, uint32_t timeout)
{
	if (osOK == osSemaphoreAcquire(serial_objs[(uint8_t)id].rx_sem, timeout)) {
		return 1;
	}
	return 0;
}

inline SERIAL_ID get_serial_id(UART_HandleTypeDef *huart)
{
	SERIAL_ID id = SERIAL_ID_MAX;
	if (huart == &huart1) {
		id = SERIAL_ID1;
	}
	else if (huart == &huart2) {
		id = SERIAL_ID2;
	}
	else if (huart == &huart3) {
		id = SERIAL_ID3;
	}
	else if (huart == &huart4) {
		id = SERIAL_ID4;
	}
	else if (huart == &huart5) {
		id = SERIAL_ID5;
	}
	else if (huart == &huart6) {
		id = SERIAL_ID6;
	}
	return id;
}

void serial_set_tx_cb(SERIAL_ID id, serial_cmplt_cb fun)
{
	serial_objs[(uint8_t)id].p_tx_cmplt_cb = fun;
}

void serial_set_rx_cb(SERIAL_ID id, serial_cmplt_cb fun)
{
	serial_objs[(uint8_t)id].p_rx_cmplt_cb = fun;
}

//////////////////////////////////////////////////////////////////////////////////

/* 数据发送完成中断处理 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	SERIAL_ID id = get_serial_id(huart);
	if (serial_objs[(uint8_t)id].p_tx_cmplt_cb) {
		serial_objs[(uint8_t)id].p_tx_cmplt_cb(&serial_objs[(uint8_t)id]);
	}
	serial_objs[(uint8_t)id].cand_send = 1;
//	osSemaphoreRelease(serial_objs[(uint8_t)id].tx_sem);
}

/* 数据接收完成中断 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	SERIAL_ID id = get_serial_id(huart);
	if (Size > 0 && serial_objs[(uint8_t)id].p_rx_cmplt_cb != (void*)0) {
		serial_objs[(uint8_t)id].rx_len = Size;
		serial_objs[(uint8_t)id].p_rx_cmplt_cb(&serial_objs[(uint8_t)id]);
	}

	HAL_UARTEx_ReceiveToIdle_DMA(huart, serial_objs[(uint8_t)id].p_rx_buff, serial_objs[(uint8_t)id].rx_buff_size);
	osSemaphoreRelease(serial_objs[(uint8_t)id].rx_sem);
}

/* 数据接收错误中断处理 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	HAL_UARTEx_RxEventCallback(huart, (huart->RxXferSize - huart->RxXferCount));
}
