#ifndef COLLECT_DATA_PROTOCOL_H_
#define COLLECT_DATA_PROTOCOL_H_

#include <stdint.h>
#include "mavlink.h"

typedef enum {
	COLLECT_PROTOCOL_MSG_ID_ACK = 0,
	COLLECT_PROTOCOL_MSG_ID_HEARTBEAT = 1,
	COLLECT_PROTOCOL_MSG_ID_CMD = 2,
	COLLECT_PROTOCOL_MSG_ID_PARAM4 = 16,
	COLLECT_PROTOCOL_MSG_ID_PARAM5 = 17,
}COLLECT_PROTOCOL_MSG_ID_ENUM;

typedef __packed struct {
	uint8_t msg_id;
	uint8_t ack;   // 0-错误; 1-成功
}ack_data_package;

typedef __packed struct {
	uint8_t collect_status;
	uint16_t collect_current_index;
	uint16_t bottle_id;
	uint32_t bottle_status;
	uint32_t vavle_status;
	uint32_t target_depth_mm;
	uint32_t current_depth_mm;
}heartbeat_data_package;

typedef __packed struct {
	uint16_t index;
	float gao_value;
	float andan_value;
	float lin_value;
	float an_value;
	uint16_t start_flag;
	uint16_t success_flag;
}param4_data_package;

typedef __packed struct {
	uint16_t index;    // 当前序号
	float ph_value;
	float cond_value;
	float do_value;
	float turb_value;
	float temp_value;
	uint16_t success_flag;
}param5_data_package;

typedef __packed struct {
	uint16_t type;
	uint16_t bottle_id;
	uint32_t depth_mm;
}cmd_data_package;

typedef __packed struct {
	uint8_t msg_id;
	__packed union {
		heartbeat_data_package heartbeat_data;
		cmd_data_package cmd_data;
		ack_data_package ack_data;
		param5_data_package param5_data;
		uint8_t data[31];
	} package;
}collect_data_frame;

void collect_protocol_param4_data_packed(mavlink_data32_t* data32, param4_data_package* param4);
void collect_protocol_param5_data_packed(mavlink_data32_t* data32, param5_data_package* param5);
void collect_protocol_ack_data_packed(mavlink_data32_t* data32, ack_data_package* ack);
void collect_protocol_heartbeat_data_packed(mavlink_data32_t* data32, heartbeat_data_package* ack);

#endif
