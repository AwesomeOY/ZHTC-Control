#include "CollectDataProtocol.h"
#include <string.h>

void collect_protocol_param4_data_packed(mavlink_data32_t* data32, param4_data_package* param4)
{

}

void collect_protocol_param5_data_packed(mavlink_data32_t* data32, param5_data_package* param5)
{
	collect_data_frame frame;
	frame.msg_id = COLLECT_PROTOCOL_MSG_ID_PARAM5;
	memcpy(&frame.package, param5, sizeof(param5_data_package));
	memcpy(data32->data, &frame, sizeof(collect_data_frame));
	data32->type = 0;
	data32->len = 32;
}

void collect_protocol_ack_data_packed(mavlink_data32_t* data32, ack_data_package* ack)
{
	collect_data_frame frame;
	frame.msg_id = COLLECT_PROTOCOL_MSG_ID_ACK;
	memcpy(&frame.package, ack, sizeof(ack_data_package));
	memcpy(data32->data, &frame, sizeof(collect_data_frame));
	data32->type = 0;
	data32->len = 32;
}

void collect_protocol_heartbeat_data_packed(mavlink_data32_t* data32, heartbeat_data_package* ack)
{
	collect_data_frame frame;
	frame.msg_id = COLLECT_PROTOCOL_MSG_ID_HEARTBEAT;
	memcpy(&frame.package, ack, sizeof(heartbeat_data_package));
	memcpy(data32->data, &frame, sizeof(collect_data_frame));
	data32->type = 0;
	data32->len = 32;
}
