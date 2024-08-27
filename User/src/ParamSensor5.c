#include "ParamSensor5.h"
#include "modbus_rtu.h"
#include "serial.h"
#include <string.h>

#define PARAM_SENSOR_BUFF_SIZE 64
static ParamSensor5 _paramSensor5;
static ParamSensor4 _paramSensor4;
static uint8_t _rx_buff[PARAM_SENSOR_BUFF_SIZE];
static uint8_t _param_uart_tx_buff[PARAM_SENSOR_BUFF_SIZE];
static uint8_t _param_uart_rx_buff[PARAM_SENSOR_BUFF_SIZE];
static uint8_t _rx_len = 0;

static int8_t _param_sensor_parse_data(void);
static void _param_sensor_receive_cmpt_cb(uint8_t* buf, uint16_t len);
static float _convert_float(float* f);  // 浮点数转换由CCDDAABB -> DDCCBBAA

/* 多参数485串口初始化 */
void param_sensor_init(void)
{
	serial_init(SERIAL_ID3, _param_uart_rx_buff, sizeof(_param_uart_rx_buff), SERIAL_DOUBLE_BUF_MODE_DISABLE);
	serial_set_tx_cb(SERIAL_ID3, NULL);
	serial_set_rx_cb(SERIAL_ID3, _param_sensor_receive_cmpt_cb);
}


////////////////////////////////////////
/* 五参数串口通信相关定义 */
static int8_t _param_sensor5_get_value(uint8_t id)
{
	uint8_t sendSize = modbus_rtu_read_reg_pack(_param_uart_tx_buff, id, 0, 6);
	if (serial_write(SERIAL_ID3, _param_uart_tx_buff, sendSize) > 0) {
		if (serial_can_read(SERIAL_ID3, 2000) && _rx_len > 0) {
			if (modbus_rtu_check_crc(_param_uart_rx_buff, _rx_len)) {
				return _param_sensor_parse_data();
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


/////////////////////////////////////////
/* 四参数串口通信相关定义 */
static int8_t _param_sensor4_start(uint8_t id)
{
	uint8_t sendSize = modbus_rtu_write_single_reg_pack(_param_uart_tx_buff, id, 40001, 1);
	if (serial_write(SERIAL_ID3, _param_uart_tx_buff, sendSize) > 0) {
		if (serial_can_read(SERIAL_ID3, 2000) && _rx_len > 0) {
			if (modbus_rtu_check_crc(_param_uart_rx_buff, _rx_len)) {
				return _param_sensor_parse_data();
			}
		}
	}
}

static int8_t _param_sensor4_get_value(uint8_t id)
{	
	// 读取测量结果
	uint8_t sendSize = modbus_rtu_read_reg_pack(_param_uart_tx_buff, id, 30001, 5);
	if (serial_write(SERIAL_ID3, _param_uart_tx_buff, sendSize) > 0) {
		if (serial_can_read(SERIAL_ID3, 2000) && _rx_len > 0) {
			if (modbus_rtu_check_crc(_param_uart_rx_buff, _rx_len)) {
				return _param_sensor_parse_data();
			}
		}
	}
	return 0;
}

static int8_t _get_param_sensor4_gao(void)
{
	return _param_sensor4_get_value(PARAM_SENSOR4_GAO_ID);
}

static int8_t _get_param_sensor4_andan(void)
{
	return _param_sensor4_get_value(PARAM_SENSOR4_AN_DAN_ID);
}

static int8_t _get_param_sensor4_lin(void)
{
	return _param_sensor4_get_value(PARAM_SENSOR4_LIN_ID);
}

static int8_t _get_param_sensor4_an(void)
{
	return _param_sensor4_get_value(PARAM_SENSOR4_AN_ID);
}

static int8_t _param_sensor_parse_data(void)
{
	union {
		float data[2];
		uint8_t buf[8];
	}float_union;
	uint8_t i = 0;
	uint16_t status = 0;
	
	// 四参数数据解析
	if (_rx_buff[0] < 128) {
		if (_rx_buff[1] == 0x06) {
			uint16_t reg = ((uint16_t)_rx_buff[2] << 8) |  (uint16_t)_rx_buff[3];
			uint16_t value = ((uint16_t)_rx_buff[4] << 8) |  (uint16_t)_rx_buff[5];
			if (reg == 40001 && value == 1) {
				return 1;
			} else {
				return 0;
			}
		} else if (_rx_buff[1] == 0x03 && _rx_buff[2] == 10) {
			status = (((uint16_t)_rx_buff[3]<<8) | _rx_buff[4]);
			if (status != 1) { // 状态寄存器，1-OK
				return 0;
			}
			
			for (i=0; i < 8; ++i) {
				float_union.buf[i] = _rx_buff[5+i];
			}
			
			switch (_param_uart_rx_buff[0]) {
				case PARAM_SENSOR4_GAO_ID:
					_paramSensor4.gao_value = _convert_float(&float_union.data[0]);
					break;
				case PARAM_SENSOR4_AN_DAN_ID:
					_paramSensor4.andan_value = _convert_float(&float_union.data[0]);
					break;
				case PARAM_SENSOR4_LIN_ID:
					_paramSensor4.lin_value = _convert_float(&float_union.data[0]);
					break;
				case PARAM_SENSOR4_AN_ID:
					_paramSensor4.an_value = _convert_float(&float_union.data[0]);
					break;
				default:
					return 0;
			}
			return 1;
		}
	}
	
	// 五参数数据解析
	if (_rx_buff[0] >= 128 && _rx_buff[1] == 0x03 && _rx_buff[2] == 12) {
		status = (((uint16_t)_rx_buff[3]<<8) | _rx_buff[4]);
		if (status != 0) { // 状态寄存器，0-OK
			return 0;
		}
		
		for (i=0; i < 8; ++i) {
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
				return 0;
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

const ParamSensor4* get_current_param_sensor4(void)
{
	return (const ParamSensor4*)&_paramSensor4;
}

void param_sensor_restart(void)
{
	_paramSensor5.success = 0;
	_paramSensor4.success = 0;
	_paramSensor5.success_flag = 0;
	_paramSensor4.success_flag = 0;
	_paramSensor4.start_flag = 0;
}

/* 轮询获取所有传感器的值 */
void param_sensor_update(void)
{
	if (_paramSensor4.success == 0) {
		if (0 == (_paramSensor4.start_flag & (1<<0))) {
			_paramSensor4.start_flag |= _param_sensor4_start(PARAM_SENSOR4_GAO_ID) ? (1<<0) : 0;
		}
		if (0 == (_paramSensor4.start_flag & (1<<1))) {
			_paramSensor4.start_flag |= _param_sensor4_start(PARAM_SENSOR4_AN_DAN_ID) ? (1<<1) : 0;
		}
		if (0 == (_paramSensor4.start_flag & (1<<2))) {
			_paramSensor4.start_flag |= _param_sensor4_start(PARAM_SENSOR4_LIN_ID) ? (1<<2) : 0;
		}
		if (0 == (_paramSensor4.start_flag & (1<<3))) {
			_paramSensor4.start_flag |= _param_sensor4_start(PARAM_SENSOR4_AN_ID) ? (1<<3) : 0;
		}
		
		if (_paramSensor4.start_flag & (1<<0)) {
			if (0 == (_paramSensor4.success_flag & (1<<0))) {
				_paramSensor4.success_flag |= _get_param_sensor4_gao() ? (1<<0) : 0;
			}			
		}
		
		if (_paramSensor4.start_flag & (1<<1)) {
			if (0 == (_paramSensor4.success_flag & (1<<1))) {
				_paramSensor4.success_flag |= _get_param_sensor4_andan() ? (1<<1) : 0;
			}
		}
		
		if (_paramSensor4.start_flag & (1<<2)) {
			if (0 == (_paramSensor4.success_flag & (1<<2))) {
				_paramSensor4.success_flag |= _get_param_sensor4_lin() ? (1<<2) : 0;
			}
		}
		
		if (_paramSensor4.start_flag & (1<<3)) {
			if (0 == (_paramSensor4.success_flag & (1<<3))) {
				_paramSensor4.success_flag |= _get_param_sensor4_an() ? (1<<3) : 0;
			}
		}
		
		if ((_paramSensor5.success_flag & 0x0F) == 0x0F) {
			_paramSensor4.success = 1;
		}
	}
		
	if (_paramSensor5.success == 0) {
		_paramSensor5.success_flag |= _get_param_sensor5_ph()   ? (1<<0) : 0;
		_paramSensor5.success_flag |= _get_param_sensor5_cond() ? (1<<1) : 0;
		_paramSensor5.success_flag |= _get_param_sensor5_do()   ? (1<<2) : 0;
		_paramSensor5.success_flag |= _get_param_sensor5_turb() ? (1<<3) : 0;
		if ((_paramSensor5.success_flag & 0x0F) == 0x0F) {
			_paramSensor5.success = 1;
		}
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
	
