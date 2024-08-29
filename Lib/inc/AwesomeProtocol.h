#ifndef AWESOME_PROTOCOL_H_
#define AWESOME_PROTOCOL_H_

#include <stdint.h>

#define AWESOME_PROTOCOL_HEADER 0x55
#define AWESOME_PROTOCOL_HEADER_SIZE 0x04
#define AWESOME_PROTOCOL_DATA_MAX_SIZE 130
#define AWESOME_PROTOCOL_MAX_SIZE (AWESOME_PROTOCOL_DATA_MAX_SIZE + AWESOME_PROTOCOL_HEADER_SIZE + 2)

typedef enum {
	UPGRADE_CMD_NONE = 0,
	UPGRADE_CMD_START,
	UPGRADE_CMD_LEN,
	UPGRADE_CMD_DATA,
	UPGRADE_CMD_CRC,
	UPGRADE_CMD_ACK,   // 1-应答成功， 0-应答失败
	UPGRADE_CMD_END
}UPGRADE_CMD_ENUM;

typedef struct {
	uint8_t upgrade_cmd;
	uint8_t size;
	uint8_t data[128];
}AwesomeProtocolUpgradeMessage;

typedef struct {
	uint8_t header;
	uint8_t seq;
	uint8_t msg_id;
	uint8_t len;
	union {
		AwesomeProtocolUpgradeMessage upgrade_msg;
		uint8_t buf[AWESOME_PROTOCOL_DATA_MAX_SIZE];
	}data;
	uint16_t crc16;
}AwesomeMessage;

typedef enum {
	AWESOME_PARSE_STATUS_NONE = 0,
	AWESOME_PARSE_STATUS_HEADER,
	AWESOME_PARSE_STATUS_SEQ,
	AWESOME_PARSE_STATUS_MSG_ID,
	AWESOME_PARSE_STATUS_LEN,
	AWESOME_PARSE_STATUS_DATA,
	AWESOME_PARSE_STATUS_CRC1,
	AWESOME_PARSE_STATUS_CRC2,
	AWESOME_PARSE_STATUS_SUCCESS
}AWESOME_PARSE_STATUS_ENUM;

typedef struct {
	AWESOME_PARSE_STATUS_ENUM status;
	uint8_t index;
	uint8_t last_seq;
}AwesomeParseStatus;

typedef enum {
	AWESOME_PROTOCOL_MSG_ID_VERSION = 0, 
	AWESOME_PROTOCOL_MSG_ID_UPGRADE = 0x10,
}AWESOME_PROTOCOL_MSG_ID_ENUM;


AWESOME_PARSE_STATUS_ENUM awesome_protocol_parse_char(uint8_t data, AwesomeMessage* msg, AwesomeParseStatus* status);

AwesomeMessage* awesome_protocol_upgrade_msg_packed(AwesomeMessage* msg, AwesomeProtocolUpgradeMessage* upgrade);

void awesome_protocol_upgrade_msg_set(AwesomeProtocolUpgradeMessage* upgrade, UPGRADE_CMD_ENUM cmd, uint8_t* buf, uint32_t len);

void awesome_protocol_low_send(AwesomeMessage* msg);

uint16_t crc16_xmodem(uint8_t *data, uint16_t length);

uint32_t crc32(uint32_t crc, uint8_t *data, uint16_t length); 

#endif
 
