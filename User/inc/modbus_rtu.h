#ifndef MODBUS_H_H
#define MODBUS_H_H

#include <stdint.h>

#define MODBUS_RTU_READ_HOLD_REG_FUN_CODE 0X03
#define MODBUS_RTU_WRITE_SINGLE_REG_FUN_CODE 0X06

uint8_t modbus_rtu_read_reg_pack(uint8_t* pack, uint8_t addr, uint16_t regAdd, uint8_t regCount);

uint8_t modbus_rtu_write_single_reg_pack(uint8_t* pack, uint8_t addr, uint16_t regAdd, uint16_t regValue);

uint8_t modbus_rtu_check_crc(const uint8_t* pack, uint8_t size);

uint8_t modbus_rtu_read_reg(const uint8_t* pack, uint8_t size, uint16_t* reg);

#endif
