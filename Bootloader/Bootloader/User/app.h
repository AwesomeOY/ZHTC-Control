#ifndef APP_H_
#define APP_H_

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "ByteQueue.h"

#define sys_run_led_toggle() HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin)
extern uint32_t led_toggle_time_ms;

void app_init(void);

void app_run(void);

void usb_task_init(void);

void usb_process_rx_callback(const uint8_t* buf, uint32_t len);

#endif
