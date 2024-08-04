#ifndef BOOT_H_
#define BOOT_H_

#include "flash_if.h"

typedef  void (*pFunction)(void);

typedef struct {
	uint32_t update;
	uint32_t size;
	uint32_t crc32;
}app_fw_flag;

typedef enum {
	APP_UPGRADE_APP1 = 0,
	APP_UPGRADE_APP2
}APP_UPGRADE_TYPE;

typedef struct {
	uint8_t start;
	APP_UPGRADE_TYPE type;
	uint32_t start_addr;
	uint32_t end_addr;
	uint32_t current_addr;
	uint32_t crc32;
}app_upgrade_struct;

const app_fw_flag* read_app_fw_flag(void);

uint8_t set_app_fw_flag(const app_fw_flag* pff);

void set_PRIMASK(unsigned int priMask);

uint8_t app_update_start(app_upgrade_struct* upgrade, APP_UPGRADE_TYPE app_type);

uint8_t app_update_flash(app_upgrade_struct* upgrade, uint32_t* buf, uint32_t len);

uint8_t app_update_end(app_upgrade_struct* upgrade, uint8_t need_update);

uint8_t app_update_check_crc32(void);

#define CLOSE_IRP()  set_PRIMASK(1)
#define OPEN_IRP()   set_PRIMASK(0)

#endif
