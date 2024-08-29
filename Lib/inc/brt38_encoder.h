#ifndef BRT38_ENCODER_H_
#define BRT38_ENCODER_H_

#include "stm32f4xx_hal.h"
#include "user_type.h"

void brt38_encoder_init(void);

float get_brt38_angle(void);

uint8_t brt38_hw_reset(void);

void brt38_send_cmpt_process(void);

void brt38_receive_cmpt_process(uint8_t len);

uint8_t brt38_set_dir(uint8_t ccw);

uint8_t brt38_get_reset_status(void);

void brt38_set_reset(void);

uint8_t brt38_is_exist(void);

#endif
