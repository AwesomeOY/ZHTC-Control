/**
  ******************************************************************************
  * @file    IAP_Main/Inc/flash_if.h 
  * @author  MCD Application Team
  * @brief   This file provides all the headers of the flash_if functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Base address of the Flash sectors  F405 */
#define ADDR_FLASH_PAGE_0     ((uint32_t)0x08000000) /* Base @ of Page 0, 16 Kbytes */
#define ADDR_FLASH_PAGE_1     ((uint32_t)0x08004000) /* Base @ of Page 1, 16 Kbytes */
#define ADDR_FLASH_PAGE_2     ((uint32_t)0x08008000) /* Base @ of Page 2, 16 Kbytes */
#define ADDR_FLASH_PAGE_3     ((uint32_t)0x0800C000) /* Base @ of Page 3, 16 Kbytes */
#define ADDR_FLASH_PAGE_4     ((uint32_t)0x08010000) /* Base @ of Page 4, 64 Kbytes */
#define ADDR_FLASH_PAGE_5     ((uint32_t)0x08020000) /* Base @ of Page 5, 128 Kbytes */
#define ADDR_FLASH_PAGE_6     ((uint32_t)0x08040000) /* Base @ of Page 6, 128 Kbytes */
#define ADDR_FLASH_PAGE_7     ((uint32_t)0x08060000) /* Base @ of Page 7, 128 Kbytes */
#define ADDR_FLASH_PAGE_8     ((uint32_t)0x08080000) /* Base @ of Page 8, 128 Kbytes */
#define ADDR_FLASH_PAGE_9     ((uint32_t)0x080A0000) /* Base @ of Page 9, 128 Kbytes */
#define ADDR_FLASH_PAGE_10    ((uint32_t)0x080C0000) /* Base @ of Page 10,128 Kbytes */
#define ADDR_FLASH_PAGE_11    ((uint32_t)0x080E0000) /* Base @ of Page 11,128 Kbytes */
/* Error code */
enum 
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR,
  FLASHIF_PROTECTION_ERRROR
};

/* protection type */  
enum{
  FLASHIF_PROTECTION_NONE         = 0,
  FLASHIF_PROTECTION_PCROPENABLED = 0x1,
  FLASHIF_PROTECTION_WRPENABLED   = 0x2,
  FLASHIF_PROTECTION_RDPENABLED   = 0x4,
};

/* protection update */
enum {
	FLASHIF_WRP_ENABLE,
	FLASHIF_WRP_DISABLE
};

#define FLASH_END_ADDR	((uint32_t)0x080FFFFF)
#define APP1_FLASH_START_ADDR ADDR_FLASH_PAGE_5    // APP1首地址，最后运行的应用程序
#define APP2_FLASH_START_ADDR ADDR_FLASH_PAGE_8    // APP2首地址，升级固件存储空间
#define APP_CHECK_START_ADDR  ADDR_FLASH_PAGE_4    // 保存是否在boot中更新固件

/* Define the address from where user application will be loaded.
   Note: this area is reserved for the IAP code                  */
#define FLASH_PAGE_STEP         0x20000           			  /* Size of page : 128 Kbytes */
#define APPLICATION_ADDRESS     (uint32_t)0x08020000      /* Start user code address: Secotor 5 */
#define FLASH_PAGE_SIZE         0x20000

/* Notable Flash addresses */
#define USER_FLASH_END_ADDRESS        0x080A0000

/* Define the user application size */
#define USER_FLASH_SIZE               ((uint32_t)0x00080000) /* Small default template application */

/* Define bitmap representing user flash area that could be write protected (check restricted to pages 8-39). */
#define FLASH_PAGE_TO_BE_PROTECTED   ((uint32_t)(0x1F)<<(16+5))


/* Exported macro ------------------------------------------------------------*/
/* ABSoulute value */
#define ABS_RETURN(x,y)               (((x) < (y)) ? (y) : (x))

/* Get the number of sectors from where the user program will be loaded */
#define FLASH_SECTOR_NUMBER           ((uint32_t)(ABS_RETURN(APPLICATION_ADDRESS,FLASH_START_BANK1))>>12)

/* Compute the mask to test if the Flash memory, where the user program will be
  loaded, is write protected */
#define FLASH_PROTECTED_SECTORS       (~(uint32_t)((1 << FLASH_SECTOR_NUMBER) - 1))
/* Exported functions ------------------------------------------------------- */
void FLASH_If_Init(void);
uint32_t FLASH_If_Erase(uint32_t sector, uint32_t saddr, uint32_t eaddr);
uint32_t FLASH_If_GetWriteProtectionStatus(void);
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length, uint32_t endAddr);
uint32_t FLASH_If_WriteProtectionConfig(uint32_t protectionstate);

#endif  /* __FLASH_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
