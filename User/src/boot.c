#include "flash_if.h"
#include "boot.h"
#include "usbd_cdc.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static app_fw_flag fw_flag;

static uint32_t _crc32(uint32_t crc, uint8_t *data, uint16_t length);

const app_fw_flag* read_app_fw_flag(void)
{
	uint32_t size = sizeof(app_fw_flag) / sizeof(uint32_t);
	uint32_t* pu = (uint32_t*)&fw_flag;
	uint8_t i = 0;
	for (i = 0; i < size; ++i) {
		pu[i] = *((uint32_t*)(APP_CHECK_START_ADDR + i * 4));
	}
	return (const app_fw_flag*)&fw_flag;
}

uint8_t set_app_fw_flag(const app_fw_flag* pff)
{
	if (FLASHIF_OK != FLASH_If_Erase(FLASH_SECTOR_4, APP_CHECK_START_ADDR, ADDR_FLASH_PAGE_5)) {
		return 0;
	}
	if (FLASHIF_OK != FLASH_If_Write(APP_CHECK_START_ADDR, (uint32_t *)pff, sizeof(app_fw_flag)/sizeof(uint32_t), FLASH_END_ADDR)) {
		return 0;
	}
	return 1;
}


/* 应用程序跳转 */
void jumpToApp(uint32_t app_addr)
{
	pFunction JumpToApplication;
	uint32_t JumpAddress;
		
	if(((*(__IO uint32_t*)app_addr) & 0x2FFE0000 ) == 0x20000000) 
	{
		/* Jump to user application */
		USBD_Stop(&hUsbDeviceFS);
		hUsbDeviceFS.pClass[0]->DeInit(&hUsbDeviceFS, hUsbDeviceFS.ConfIdx);
//		HAL_TIM_Base_DeInit(&htim1);

		JumpAddress = *(__IO uint32_t*) (app_addr + 4);
		JumpToApplication = (pFunction) JumpAddress;

		HAL_DeInit();
		SCB->VTOR = FLASH_BASE | FLASH_PAGE_STEP;
		
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) app_addr);
		CLOSE_IRP();
		
		JumpToApplication();
	}
}

__inline void set_PRIMASK(unsigned int priMask)
{
  register unsigned int __regPriMask         __asm("primask");
  __regPriMask = (priMask);
}

uint8_t app_update_start(app_upgrade_struct* upgrade, APP_UPGRADE_TYPE app_type)
{
	uint32_t sector = 0;
	upgrade->start = 1;
	if (app_type == APP_UPGRADE_APP1) {
		upgrade->start_addr = APP1_FLASH_START_ADDR;
		upgrade->end_addr = APP1_FLASH_START_ADDR + FLASH_PAGE_SIZE * 3;
		upgrade->current_addr = APP1_FLASH_START_ADDR;
		sector = 5;
	} else if (app_type == APP_UPGRADE_APP2) {
		upgrade->start_addr = APP2_FLASH_START_ADDR;
		upgrade->end_addr = APP2_FLASH_START_ADDR + FLASH_PAGE_SIZE * 3;
		upgrade->current_addr = APP2_FLASH_START_ADDR;
		sector = 8;
	} else {
		upgrade->start = 0;
		return 0;
	}
	if (FLASHIF_OK == FLASH_If_Erase(sector, upgrade->start_addr, upgrade->end_addr)) {
		return 1;
	}
	return 0;
}

uint8_t app_update_flash(app_upgrade_struct* upgrade, uint32_t* buf, uint32_t len)
{
	if (upgrade->start) {
		upgrade->crc32 = _crc32(upgrade->crc32, (uint8_t*)buf, len * 4);
		if (FLASHIF_OK == FLASH_If_Write(upgrade->current_addr, buf, len, upgrade->end_addr)) {
			upgrade->current_addr += 4 * len;
			return 1;
		}
	}
	return 0;
}

uint8_t app_update_end(app_upgrade_struct* upgrade, uint8_t need_update)
{
	if (upgrade->start) {
		upgrade->start = 0;
		if (need_update) {
			app_fw_flag fw;
			fw.update = 1;
			fw.size = upgrade->current_addr - upgrade->start_addr;
			fw.crc32 = upgrade->crc32;
			if (set_app_fw_flag(&fw)) {
				// 跳转到bootloader
				return 1;
			} else {
				return 0;
			}
		}
		return 1;		
	}
	return 1;
}

static uint32_t _crc32(uint32_t crc, uint8_t *data, uint16_t length)
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

uint8_t app_update_check_crc32(void)
{
	uint32_t crc = 0;
	uint32_t i = 0;
	uint32_t data = 0;
	const app_fw_flag* paff = read_app_fw_flag();
	uint32_t len = paff->size / 4;
	
	for (i = 0; i < len; ++i) {
		data = *((uint32_t*)(APP2_FLASH_START_ADDR+4*i));
		crc = _crc32(crc, (uint8_t*)&data, 4);
	}
	if (crc == paff->crc32) {
		return 1;
	}
	return 0;
}

uint8_t app_update_check(void)
{
	app_fw_flag fw = *(read_app_fw_flag());
	if (fw.update) {
		if (app_update_check_crc32()) {
			app_upgrade_struct upgrade;
			if (app_update_start(&upgrade, APP_UPGRADE_APP1)) {
				if (app_update_flash(&upgrade, (uint32_t*)APP2_FLASH_START_ADDR, fw.size/4)) {
					fw.update = 0;
					if (set_app_fw_flag(&fw))
						return 1;
				}
			}
		}
	}
	return 0;
}
