#include "app.h"
#include "mavlink.h"
#include "usbd_cdc_if.h"

static uint8_t _mavlink_serial_rec_buf[2*80];
static uint8_t _mavlink_serial_send_buf[256];
static uint8_t _mavlink_rec_buff[3096];
static uint8_t _mavlink_send_buff[2048];
static Queue _mavlink_rec_queue;
static Queue _mavlink_send_queue;

static mavlink_system_t _uav_mavlink_system = {0, 0};   // 飞控端ID
mavlink_system_t mavlink_system = {168, MAV_COMP_ID_USER1}; // 本机ID

mavlink_position_target_global_int_t position_global;
mavlink_gps_raw_int_t gps_raw;
mavlink_attitude_t attitude;
mavlink_sys_status_t uav_status;
mavlink_command_ack_t ack;
mavlink_data32_t rec_data32;

static void _mavlink_request_msg(void);
void serial_back_test(void);

/* 串口接收数据回调函数，将接收到数据放到队列中存储 */
static inline void _mavlink_serial_rx_cb(uint8_t* buf, uint16_t size)
{
	int i = 0;
	for ( ; i < size; ++i) {
		queue_en(&_mavlink_rec_queue, buf[i]);
	}	
}

void mavlink_init(void)
{
	queue_init(&_mavlink_rec_queue, _mavlink_rec_buff, sizeof(_mavlink_rec_buff));
	queue_init(&_mavlink_send_queue, _mavlink_send_buff, sizeof(_mavlink_send_buff));
	serial_set_rx_cb(SERIAL_ID1, _mavlink_serial_rx_cb);
	serial_init(SERIAL_ID1, _mavlink_serial_rec_buf, sizeof(_mavlink_serial_rec_buf), SERIAL_DOUBLE_BUF_MODE_ENABLE);	
	mavlink_status_t *status = mavlink_get_channel_status(SERIAL_ID1);
	
	const osThreadAttr_t mavlinkTask_attributes = {
	  .name = "mavTask",
	  .stack_size = 128 * 4,
	  .priority = (osPriority_t) osPriorityNormal,
	};
	
	osThreadNew(mavlink_process_task, NULL, &mavlinkTask_attributes);
}

/* Mavlink通信底层发送接口 */
void mavlink_send_buff(uint8_t id, uint8_t* buf, uint32_t len)
{
	if (id == SERIAL_ID1) {
		int i = 0;
		for (i = 0; i < len; ++i) {
			queue_en(&_mavlink_send_queue, buf[i]);
		}
	}
}

/* 解析Mavlink数据包 */
static void _mavlink_parse(void)
{
	static mavlink_message_t msg;
	static mavlink_status_t msg_status;
	int i = 0;
	int rec_len = queue_len(&_mavlink_rec_queue);
	
	for (i = 0; i < rec_len; ++i) {
		uint8_t data = 0;
		if (queue_de(&_mavlink_rec_queue, &data)) {
			if (MAVLINK_FRAMING_OK == mavlink_frame_char(SERIAL_ID1, data, &msg, &msg_status)) {
				switch (msg.msgid) {					
					case MAVLINK_MSG_ID_HEARTBEAT:
						if (_uav_mavlink_system.sysid==0 && _uav_mavlink_system.compid == 0) {
							_uav_mavlink_system.sysid = 1;
							_uav_mavlink_system.compid = 1;
						}
						break;
					case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
						mavlink_msg_position_target_global_int_decode(&msg, &position_global);
						break;
					case MAVLINK_MSG_ID_GPS_RAW_INT:
						mavlink_msg_gps_raw_int_decode(&msg, &gps_raw);
						break;
					case MAVLINK_MSG_ID_ATTITUDE:
						mavlink_msg_attitude_decode(&msg, &attitude);
						break;
					case MAVLINK_MSG_ID_SYS_STATUS:
						mavlink_msg_sys_status_decode(&msg, &uav_status);
						break;
					case MAVLINK_MSG_ID_COMMAND_ACK:
					{
						mavlink_msg_command_ack_decode(&msg, &ack);
						break;
					}
					case MAVLINK_MSG_ID_DATA32:
					{
						mavlink_msg_data32_decode(&msg, &rec_data32);						
						collect_protocol_parse(&rec_data32);
						break;
					}
					default:
						break;
				}
			}
		}
		else {
			break;
		}
	}
}

/* Mavlink通信处理任务 发送和接收 */
void mavlink_process_task(void* arg)
{
	uint32_t _last_send_time_ms = 0;
	while (1) {
		uint32_t now = osKernelGetTickCount();
		
		if (now - _last_send_time_ms >= 5) {
			_last_send_time_ms = now;
			
			// 自动消息请求
			_mavlink_request_msg();
			
			// 发送mavlink数据
			if (serial_can_write(SERIAL_ID1, 10)) {
				int send_len = queue_len(&_mavlink_send_queue);
				int i = 0;
				send_len = (send_len >= sizeof(_mavlink_serial_send_buf)) ? sizeof(_mavlink_serial_send_buf) : send_len;
				if (send_len > 0) {
					for (i = 0; i < send_len; ++i) {
						queue_de(&_mavlink_send_queue, &_mavlink_serial_send_buf[i]);
					}
					serial_write(SERIAL_ID1, _mavlink_serial_send_buf, send_len);
				}
			}
			
			// mavkink数据解析处理
			_mavlink_parse();
			//serial_back_test();
			osDelay(1);
		}
	}
}

/* Mavlink请求mavlink数据信息，每隔两秒主动发送一次 */
static void _mavlink_request_msg(void)
{
	static uint32_t _last_time_ms = 0;
	static uint32_t _last_heabeat_ms = 0;
	static uint32_t _last_collect_time_ms = 0;
//	mavlink_request_data_stream_t stream;
	uint32_t now = osKernelGetTickCount();
	if (now - _last_time_ms >= 5000) {
		_last_time_ms = now;		
		
		mavlink_command_long_t commd_long;
		memset(&commd_long, 0, sizeof(commd_long));
		
		if (_uav_mavlink_system.sysid == 0 && _uav_mavlink_system.compid == 0) {
			return;
		}
		
		commd_long.target_system = _uav_mavlink_system.sysid;
		commd_long.target_component = 0;
		commd_long.confirmation = 0;
		commd_long.param7 = 0;
		
		commd_long.command = MAV_CMD_SET_MESSAGE_INTERVAL;
		commd_long.param1 = MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT;  // 全局位置
		commd_long.param2 = 500000;  // 500ms发送一次
		mavlink_msg_command_long_send_struct((mavlink_channel_t)SERIAL_ID1, &commd_long);
		
		commd_long.param1 = MAVLINK_MSG_ID_GPS_RAW_INT;  // GPS原始数据
		commd_long.param2 = 500000;   // 500ms发一次
		mavlink_msg_command_long_send_struct((mavlink_channel_t)SERIAL_ID1, &commd_long);
		
		commd_long.param1 = MAVLINK_MSG_ID_ATTITUDE;     // 姿态数据
		commd_long.param2 = 200000;   // 200ms发一次
		mavlink_msg_command_long_send_struct((mavlink_channel_t)SERIAL_ID1, (const mavlink_command_long_t*)&commd_long);
		
		commd_long.param1 = MAVLINK_MSG_ID_SYS_STATUS;   // 系统状态标志
		commd_long.param2 = 500000;   // 500ms发一次
		mavlink_msg_command_long_send_struct((mavlink_channel_t)SERIAL_ID1, &commd_long);
		
//		stream.target_system = 0x01;
//		stream.target_component = _uav_mavlink_system.compid;
//		stream.req_message_rate = 0;
//		stream.start_stop = 0;
//		stream.req_stream_id = MAV_DATA_STREAM_ALL;
//		mavlink_msg_request_data_stream_send_struct((mavlink_channel_t)SERIAL_ID1, (const mavlink_request_data_stream_t*)&stream);
	}

	if (now - _last_heabeat_ms >= 1000){
		mavlink_heartbeat_t hbt;
		hbt.autopilot = 0;
		hbt.base_mode = 0;
		hbt.custom_mode = 0;
		hbt.mavlink_version = 3;
		_last_heabeat_ms = now;
		mavlink_msg_heartbeat_send_struct((mavlink_channel_t)SERIAL_ID1, (const mavlink_heartbeat_t*)&hbt);
		collect_protocol_send_heartbeat();		
	}
	
	if (now - _last_collect_time_ms >= 1500) {
		collect_protocol_send_param5();
		collect_protocol_send_param4();
		_last_collect_time_ms = now;
	}
}

void serial_back_test(void)
{
	uint8_t data = 0;
	int i = 0;
	int rec_len = queue_len(&_mavlink_rec_queue);
	
	if (rec_len > 0) {
		for (i = 0; i < rec_len; ++i) {
			queue_de(&_mavlink_rec_queue, &data);
			
			queue_en(&_mavlink_send_queue, data);
		}
	}
}
