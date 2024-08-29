#ifndef SERIAL_H_
#define SERIAL_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

typedef void (*serial_tx_cmplt_cb)(void* arg);
typedef void (*serial_rx_cmplt_cb)(uint8_t* buf, uint16_t size);

typedef enum {
	SERIAL_ID1 = 0,
	SERIAL_ID2,
	SERIAL_ID3,
	SERIAL_ID4,
	SERIAL_ID5,
	SERIAL_ID6,
	SERIAL_ID_MAX
}SERIAL_ID;

typedef enum {
	SERIAL_DOUBLE_BUF_MODE_DISABLE = 0,
	SERIAL_DOUBLE_BUF_MODE_ENABLE = 1
}SERIAL_DOUBLE_BUF_MODE;

typedef struct {
	uint16_t len;
	uint8_t* buff;
}serial_rx_bf;

typedef struct {
	uint8_t init : 1;
	uint8_t cand_send : 1;
	uint8_t double_mode_enable : 1;
	uint8_t double_buf_index;
	uint16_t tx_buff_size;
	uint16_t rx_buff_size;
	uint16_t rx_len;
	UART_HandleTypeDef* huart;
	osSemaphoreId_t tx_sem;
	osSemaphoreId_t rx_sem;	
	uint8_t* p_tx_buff;
	uint8_t* p_rx_buff;
	serial_tx_cmplt_cb p_tx_cmplt_cb;
	serial_rx_cmplt_cb p_rx_cmplt_cb;
}serial_obj;

void serial_init(SERIAL_ID id, uint8_t* rbuf, uint16_t rlen, SERIAL_DOUBLE_BUF_MODE buf_mode);

uint16_t serial_write(SERIAL_ID id, uint8_t* pbuf, uint16_t len);

uint8_t serial_can_read(SERIAL_ID id, uint32_t timeout);

uint8_t serial_can_write(SERIAL_ID id, uint32_t timeout);

uint16_t serial_read(SERIAL_ID id, uint8_t* pbuf, uint16_t len);

SERIAL_ID get_serial_id(UART_HandleTypeDef *huart);

void serial_set_tx_cb(SERIAL_ID id, serial_tx_cmplt_cb fun);

void serial_set_rx_cb(SERIAL_ID id, serial_rx_cmplt_cb fun);

#endif
