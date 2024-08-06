#include "ParamSensor5.h"
#include "modbus_rtu.h"
#include "serial.h"
#include <string.h>

#define PARAM_SENSOR_BUFF_SIZE 64
static ParamSensor5 _paramSensor5;
static uint8_t _rx_buff[PARAM_SENSOR_BUFF_SIZE];
static uint8_t _param_uart_tx_buff[PARAM_SENSOR_BUFF_SIZE];
static uint8_t _param_uart_rx_buff[PARAM_SENSOR_BUFF_SIZE];
static uint8_t _rx_len = 0;

static int8_t _param_sensor5_parse_data(void);
static void _param_sensor_receive_cmpt_cb(uint8_t* buf, uint16_t len);
static float _convert_float(float* f);  // 浮点数转换由CCDDAABB -> DDCCBBAA

/* 多参数485串口初始化 */
void param_sensor5_init(void)
{
	serial_init(SERIAL_ID3, _param_uart_rx_buff, sizeof(_param_uart_rx_buff), SERIAL_DOUBLE_BUF_MODE_DISABLE);
	serial_set_tx_cb(SERIAL_ID3, NULL);
	serial_set_rx_cb(SERIAL_ID3, _param_sensor_receive_cmpt_cb);
}

static int8_t _param_sensor5_get_value(uint8_t id)
{
	uint8_t sendSize = modbus_rtu_read_reg_pack(_param_uart_tx_buff, id, 0, 6);
	if (serial_write(SERIAL_ID3, _param_uart_tx_buff, sendSize) > 0) {
		if (serial_can_read(SERIAL_ID3, 2000) && _rx_len > 0) {
			if (modbus_rtu_check_crc(_param_uart_rx_buff, _rx_len)) {
				return _param_sensor5_parse_data();
			}
		}
	}
	return 0;
}

static int8_t _get_param_sensor5_ph(void)
{
	return _param_sensor5_get_value(PARAM_SENSOR5_PH_ID);
}

static int8_t _get_param_sensor5_cond(void)
{
	return _param_sensor5_get_value(PARAM_SENSOR5_COND_ID);
}

static int8_t _get_param_sensor5_do(void)
{
	return _param_sensor5_get_value(PARAM_SENSOR5_DO_ID);
}

static int8_t _get_param_sensor5_turb(void)
{
	return _param_sensor5_get_value(PARAM_SENSOR5_TURB_ID);
}

static int8_t _param_sensor5_parse_data(void)
{
	union {
		float data[2];
		uint8_t buf[8];
	}float_union;
	
	if (_rx_buff[1] == 0x03 && _rx_buff[2] == 12) {
		uint8_t i = 0;
		uint16_t status = (((uint16_t)_rx_buff[3]<<8) | _rx_buff[4]);
		if (status != 0) { // 状态寄存器，0-OK
			return 0;
		}
		
		for ( ; i < 8; ++i) {
			float_union.buf[i] = _rx_buff[7+i];
		}		
		
		switch (_param_uart_rx_buff[0]) {
			case PARAM_SENSOR5_PH_ID:
				_paramSensor5.temp_value = _convert_float(&float_union.data[1]);
				_paramSensor5.ph_value = _convert_float(&float_union.data[0]);
				break;
			case PARAM_SENSOR5_COND_ID:
				_paramSensor5.cond_value = _convert_float(&float_union.data[0]);
				break;
			case PARAM_SENSOR5_DO_ID:
				_paramSensor5.do_value = _convert_float(&float_union.data[0]);
				break;
			case PARAM_SENSOR5_TURB_ID:
				_paramSensor5.turb_value = _convert_float(&float_union.data[0]);
				break;
			default:
				break;
		}
		return 1;
	}
	return 0;
}


/* 串口接收数据完成中断，关闭485接收，开启485发送 */
static void _param_sensor_receive_cmpt_cb(uint8_t* buf, uint16_t len)
{
	if (len > 0) {
		_rx_len = len;
		memcpy(_rx_buff, buf, len);
	}	
}

const ParamSensor5* get_current_param_sensor5(void)
{
	return (const ParamSensor5*)&_paramSensor5;
}

void param_sensor5_restart(void)
{
	_paramSensor5.success = 0;
}

/* 轮询获取所有传感器的值 */
void param_sensor5_update(void)
{
	uint8_t flag = 0;
	flag |= _get_param_sensor5_ph()   ? (1<<0) : 0;
	flag |= _get_param_sensor5_cond() ? (1<<1) : 0;
	flag |= _get_param_sensor5_do()   ? (1<<2) : 0;
	flag |= _get_param_sensor5_turb() ? (1<<3) : 0;
	if ((flag & 0x0F) == 0x0F) {
		_paramSensor5.success = 1;
	}
}

/* 将接收到的浮点数转换成本平台浮点数
** (设备存储格式)AABBCCDD -> (传输格式先后顺序)CCDDAABB -> (本平台格式)DDCCBBAA
*/
static float _convert_float(float* f)
{
	uint8_t* data = (uint8_t*)f;
	uint8_t temp = data[0];
	data[0] = data[1];
	data[1] = temp;
	temp = data[2];
	data[2] = data[3];
	data[3] = temp;
	return *f;
}
	
