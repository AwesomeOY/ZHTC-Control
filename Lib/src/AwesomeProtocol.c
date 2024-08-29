#include "AwesomeProtocol.h"

AWESOME_PARSE_STATUS_ENUM awesome_protocol_parse_char(uint8_t data, AwesomeMessage* msg, AwesomeParseStatus* status)
{
	switch (status->status) {
		case AWESOME_PARSE_STATUS_NONE:
			if (data == AWESOME_PROTOCOL_HEADER) {				
				msg->header = data;
				status->index = 0;
				status->status = AWESOME_PARSE_STATUS_HEADER;
			}
			break;
			
		case AWESOME_PARSE_STATUS_HEADER:
			msg->seq = data;
			status->status = AWESOME_PARSE_STATUS_SEQ;
			break;
			
		case AWESOME_PARSE_STATUS_SEQ:
			msg->msg_id = data;
			status->status = AWESOME_PARSE_STATUS_MSG_ID;		
			break;
			
		case AWESOME_PARSE_STATUS_MSG_ID:
			if (msg->len > AWESOME_PROTOCOL_DATA_MAX_SIZE)
			{
				status->status = AWESOME_PARSE_STATUS_NONE;
				break;
			}
			msg->len = data;
			status->status = AWESOME_PARSE_STATUS_LEN;
			break;
			
		case AWESOME_PARSE_STATUS_LEN:
		case AWESOME_PARSE_STATUS_DATA:
			msg->data.buf[status->index++] = data;
			if (status->index >= msg->len) {
				status->status = AWESOME_PARSE_STATUS_CRC1;
			}
			break;		
		case AWESOME_PARSE_STATUS_CRC1:
			msg->crc16 = data;
			status->status = AWESOME_PARSE_STATUS_CRC2;
			break;
		case AWESOME_PARSE_STATUS_CRC2:
			msg->crc16 |= ((uint16_t)(data) << 8);
			status->status = AWESOME_PARSE_STATUS_NONE;
			if (crc16_xmodem((uint8_t*)msg, AWESOME_PROTOCOL_HEADER_SIZE + msg->len) == msg->crc16) {
				return AWESOME_PARSE_STATUS_SUCCESS;
			}
			break;
		default:
			status->status = AWESOME_PARSE_STATUS_NONE;
			break;
	}
	return AWESOME_PARSE_STATUS_NONE;
}

AwesomeMessage* awesome_protocol_upgrade_msg_packed(AwesomeMessage* msg, AwesomeProtocolUpgradeMessage* upgrade)
{
	uint8_t i = 0;
	
	msg->header = AWESOME_PROTOCOL_HEADER;
	msg->seq += 1;
	msg->len = upgrade->size + 2;
	msg->msg_id = AWESOME_PROTOCOL_MSG_ID_UPGRADE;
	
	for ( ; i < msg->len; ++i) {
		msg->data.buf[i] = *(((uint8_t*)upgrade)+i);
	}
	
	msg->crc16 = crc16_xmodem((uint8_t*)msg, msg->len + AWESOME_PROTOCOL_HEADER_SIZE);
	return msg;
}

void awesome_protocol_upgrade_msg_set(AwesomeProtocolUpgradeMessage* upgrade, UPGRADE_CMD_ENUM cmd, uint8_t* buf, uint32_t len)
{
	uint32_t i = 0;
	
	upgrade->upgrade_cmd = cmd;
	upgrade->size = len;
	
	for ( ; i < len; ++i) {
		upgrade->data[i] = buf[i];
	}
}

/******************************************************************************
 * Name:    CRC-16/XMODEM       x16+x12+x5+1
 * Poly:    0x1021
 * Init:    0x0000
 * Refin:   False
 * Refout:  False
 * Xorout:  0x0000
 * Alias:   CRC-16/ZMODEM,CRC-16/ACORN
 *****************************************************************************/
uint16_t crc16_xmodem(uint8_t *data, uint16_t length)
{
    uint8_t i;
    uint16_t crc = 0;            // Initial value
    while(length--)
    {
        crc ^= (uint16_t)(*data++) << 8; // crc ^= (uint16_t)(*data)<<8; data++;
        for (i = 0; i < 8; ++i)
        {
            if ( crc & 0x8000 )
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/******************************************************************************
 * Name:    CRC-32  x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1
 * Poly:    0x4C11DB7
 * Init:    0xFFFFFFF
 * Refin:   True
 * Refout:  True
 * Xorout:  0xFFFFFFF
 * Alias:   CRC_32/ADCCP
 * Use:     WinRAR,ect.
 *****************************************************************************/
uint32_t crc32(uint32_t crc, uint8_t *data, uint16_t length)
{
    uint8_t i;
    crc = ~crc; 
    while(length--)
    {
        crc ^= *data++;                // crc ^= *data; data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;// 0xEDB88320= reverse 0x04C11DB7
            else
                crc = (crc >> 1);
        }
    }
    return ~crc;
}

extern uint32_t usb_process_write(const uint8_t* buf, uint32_t len);
void awesome_protocol_low_send(AwesomeMessage* msg)
{
	usb_process_write((const uint8_t*)msg, AWESOME_PROTOCOL_HEADER_SIZE + msg->len);
	usb_process_write((const uint8_t*)&(msg->crc16), 2);
}

