#include "modbus_rtu.h"

static uint32_t _crc16(const uint8_t* pbuf, uint8_t num)
{
	int i,j;
	uint32_t wcrc=0xffff;
	
	for (i = 0;i < num; i++)
	{
		wcrc ^= (uint32_t)pbuf[i];
		for (j = 0; j < 8; j++)
		{
			if(wcrc & 0x0001)
			{
				wcrc >>= 1;
				wcrc ^= 0xa001;
			}
			else
				wcrc >>= 1;
		}
	}
	return wcrc;
}

/* 读寄存器值 */
uint8_t modbus_rtu_read_reg_pack(uint8_t* pack, uint8_t addr, uint16_t regAdd, uint8_t regCount)
{
	uint16_t crc = 0;
	
	if (regCount == 0)
		return 0;
	
	pack[0] = addr;
	pack[1] = MODBUS_RTU_READ_HOLD_REG_FUN_CODE;
	
	pack[2] = (regAdd >> 8) & 0xFF;
	pack[3] = regAdd & 0xFF;
	
	pack[4] = (regCount >> 8) & 0xFF;
	pack[5] = regCount & 0xFF;
	
	crc = _crc16(pack, 6);	
	pack[6] = crc & 0xFF;
	pack[7] = (crc >> 8) & 0xFF;
	
	return 8;
}

/* 读单个寄存器值 */
uint8_t modbus_rtu_write_single_reg_pack(uint8_t* pack, uint8_t addr, uint16_t regAdd, uint16_t regValue)
{
	uint16_t crc = 0;
	
	pack[0] = addr;
	pack[1] = MODBUS_RTU_WRITE_SINGLE_REG_FUN_CODE;
	
	pack[2] = (regAdd>> 8) & 0xFF;
	pack[3] = regAdd & 0xFF;
	
	pack[4] = (regValue>> 8) & 0xFF;
	pack[5] = regValue & 0xFF;
	
	crc = _crc16(pack, 6);
	pack[6] = crc & 0xFF;
	pack[7] = (crc >> 8) & 0xFF;		
	
	return 8;
}

/* modbus RTU数据包CRC检查 */
uint8_t modbus_rtu_check_crc(const uint8_t* pack, uint8_t size)
{
	uint16_t crc = 0;
	if (size < 2)
		return 0;
	crc = _crc16(pack, size-2);
	if (crc == (uint16_t)(pack[size-2] | ((uint16_t)(pack[size-1]) << 8)))
	{
		return 1;
	}
	return 0;
}

/* 读寄存器值 */
uint8_t modbus_rtu_read_reg(const uint8_t* pack, uint8_t size, uint16_t* regValue)
{
	uint8_t i = 0;
	if (pack[1] != MODBUS_RTU_READ_HOLD_REG_FUN_CODE || regValue == (void*)0 || size < 8 || pack[2] % 2 != 0)
		return 0;
	
	for (i = 0; i < pack[2]; i += 2) {
		regValue[i/2] = ((uint16_t)pack[3+i] << 8) | pack[4+i];
	}
	
	return pack[2] / 2;	
}
