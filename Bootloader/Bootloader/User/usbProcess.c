#include "app.h"
#include "usbd_cdc.h"
#include "AwesomeProtocol.h"
#include "boot.h"
#include "cmsis_os2.h"
#include "boot.h"

AwesomeMessage awesomeMessage;
AwesomeMessage sendAwesomeMessage;
AwesomeParseStatus awesomeParseStatus;
AwesomeProtocolUpgradeMessage upgradeMessage;
app_upgrade_struct app_upgrade;

typedef struct {
	uint8_t init;
	uint8_t lowrxBuff[512];
	uint8_t lowtxbuff[512];
	Queue txqueue;
	Queue rxqueue;
}USB_process_class;

typedef struct {
	uint32_t max_len;
	uint32_t rec_len;
	uint32_t crc32;
}upgrade_data;

extern USBD_HandleTypeDef hUsbDeviceFS;
USB_process_class usb_process;
uint8_t txbuff[4096];
uint8_t rxbuff[4096];

const osThreadAttr_t usbSensorTask_attributes = {
  .name = "USB_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t)osPriorityLow,
};

static void usb_task(void* arg);

/* USB资源初始化 */
static void usb_process_init(void)
{
	queue_init(&usb_process.txqueue, txbuff, 4096);
	queue_init(&usb_process.rxqueue, rxbuff, 4096);
	usb_process.init = 1;
	
	osThreadNew(usb_task, NULL, &usbSensorTask_attributes);
}

/* 判断USB接收缓冲区中是否有数据 */
static uint32_t usb_process_can_read(void)
{
	if (!usb_process.init) return 0;
	return queue_len(&usb_process.rxqueue);
}

/* USB读取缓冲区中的数据 */
uint32_t usb_process_read(uint8_t* buf, uint32_t len)
{
	uint32_t reslen = usb_process_can_read();
	len = len > reslen ? reslen : len;
	if (usb_process.init && len > 0 && buf)
	{
		uint32_t i = 0;
		for ( ; i < len; ++i) {
			queue_de(&usb_process.rxqueue, &buf[i]);
		}
	}
	return len;
}

/* 读取USB发送缓冲区中的数据 */
uint32_t usb_process_read_tx_buf(uint8_t* buf, uint32_t len)
{
	uint32_t reslen = queue_len(&usb_process.txqueue);
	len = len > reslen ? reslen : len;
	if (usb_process.init && len > 0 && buf)
	{
		uint32_t i = 0;
		for ( ; i < len; ++i) {
			queue_de(&usb_process.txqueue, &buf[i]);
		}
	}
	return len;
}

/* USB应用层发送数据，将发送的数据存储到发送缓冲区 */
uint32_t usb_process_write(const uint8_t* buf, uint32_t len)
{
	uint32_t reslen = 0;
	if (usb_process.init && len > 0 && buf)
	{
		uint32_t i = 0;
		for ( ; i < len; ++i) {
			++reslen;
			if (queue_en(&usb_process.txqueue, buf[i]) == 0) {
				break;
			}
		}		
	}
	return reslen;
}

/* 发送协议应答 */
static void upgrade_send_ack(uint8_t ack)
{
	awesome_protocol_upgrade_msg_set(&upgradeMessage, UPGRADE_CMD_ACK, &ack, 1);
	awesome_protocol_upgrade_msg_packed(&sendAwesomeMessage, &upgradeMessage);
	awesome_protocol_low_send(&sendAwesomeMessage);
}

/* USB数据缓冲区发送循环 */
static void usb_process_tx_loop(void)
{
	uint8_t result = USBD_OK;
	uint32_t len = 0;
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState == USBD_OK && usb_process.init){		
		len = usb_process_read_tx_buf(usb_process.lowtxbuff, 512);
		if (len > 0)
		{
			USBD_CDC_SetTxBuffer(&hUsbDeviceFS, usb_process.lowtxbuff, len);
			USBD_CDC_TransmitPacket(&hUsbDeviceFS);
		}
		
	}
}

/* USB用户协议数据解析 */
static void usb_data_parse(void)
{
	uint32_t len = usb_process_read(usb_process.lowrxBuff, 512);
	uint8_t ack = 0;
	static upgrade_data _upgrade_data;
	if (len > 0) {
		uint32_t i = 0;
		for (i = 0; i < len; ++i) {
			if (AWESOME_PARSE_STATUS_SUCCESS == awesome_protocol_parse_char(usb_process.lowrxBuff[i], &awesomeMessage, &awesomeParseStatus)) {
				switch (awesomeMessage.msg_id)
				{
					case AWESOME_PROTOCOL_MSG_ID_UPGRADE:
						switch (awesomeMessage.data.upgrade_msg.upgrade_cmd) {
							case UPGRADE_CMD_START:
								if (app_update_start(&app_upgrade, APP_UPGRADE_APP2)) {
									ack = 1;
								}
								upgrade_send_ack(ack);
								break;
							case UPGRADE_CMD_LEN:
								awesomeParseStatus.last_seq = awesomeMessage.seq;
								_upgrade_data.crc32 = 0;
								_upgrade_data.rec_len = 0;							
								_upgrade_data.max_len = ((uint32_t)awesomeMessage.data.upgrade_msg.data[0]) | 
														((uint32_t)awesomeMessage.data.upgrade_msg.data[1] << 8) |
													    ((uint32_t)awesomeMessage.data.upgrade_msg.data[2] << 16) |
														((uint32_t)awesomeMessage.data.upgrade_msg.data[3] << 24);
								ack = 1;
								upgrade_send_ack(ack);
								break;
							case UPGRADE_CMD_DATA:
								if (((awesomeParseStatus.last_seq + 1) % 256) == awesomeMessage.seq) {
									awesomeParseStatus.last_seq = awesomeMessage.seq;
									_upgrade_data.rec_len += awesomeMessage.data.upgrade_msg.size;
									if (_upgrade_data.rec_len > _upgrade_data.max_len) {
										led_toggle_time_ms = 500;
										ack = 0;
									}
									_upgrade_data.crc32 = crc32(_upgrade_data.crc32, &(awesomeMessage.data.upgrade_msg.data[0]), awesomeMessage.data.upgrade_msg.size);
									if (app_update_flash(&app_upgrade, (uint32_t*)(&(awesomeMessage.data.upgrade_msg.data[0])), awesomeMessage.data.upgrade_msg.size/4)) {
										ack = 1;
									}
								} else if (awesomeParseStatus.last_seq == awesomeMessage.seq) {
									ack = 1;
								}								
								upgrade_send_ack(ack);
								break;
							case UPGRADE_CMD_CRC:
							{
								uint32_t crc = 	((uint32_t)awesomeMessage.data.upgrade_msg.data[0]) | 
												((uint32_t)awesomeMessage.data.upgrade_msg.data[1] << 8) |
												((uint32_t)awesomeMessage.data.upgrade_msg.data[2] << 16) |
												((uint32_t)awesomeMessage.data.upgrade_msg.data[3] << 24);
								if (crc == app_upgrade.crc32) {
									ack = 1;								
								}
								upgrade_send_ack(ack);
								break;
							}
							case UPGRADE_CMD_END:
								if (app_update_end(&app_upgrade, 1)) {
									ack = 1;
								}
								upgrade_send_ack(ack);
								break;
							default:
								break;
						}
						led_toggle_time_ms = (ack == 0) ? 200 : 50;
						break;
				}
			} 
		}
		
	}
}

static void usb_task(void* arg)
{
	app_update_check();
	while (1)
	{
		usb_data_parse();
		usb_process_tx_loop();
		osDelay(1);
	}
}

/* USB接收数据中断回调 */
void usb_process_rx_callback(const uint8_t* buf, uint32_t len)
{
	if (buf && len>0)
	{
		uint32_t i = 0;
		for ( ; i < len; ++i) {
			if (queue_en(&usb_process.rxqueue, buf[i]) == 0) {
				break;
			}
		}
	}
}

/* USB相关任务初始化 */
void usb_task_init(void)
{
	usb_process_init();
}
