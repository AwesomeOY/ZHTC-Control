#ifndef PARAM_SENSOR5_H
#define PARAM_SENSOR5_H

#include <stdint.h>

#define PARAM_SENSOR5_COMMON_ID 128
#define PARAM_SENSOR5_PH_ID   (0x01 + 128)  // PH传感器 ID
#define PARAM_SENSOR5_COND_ID (0x02 + 128)  // 电导率传感器 ID
#define PARAM_SENSOR5_DO_ID   (0x03 + 128)  // 溶解氧传感器ID
#define PARAM_SENSOR5_TURB_ID (0x04 + 128)  // 浊度传感器 ID

#define PARAM_SENSOR4_GAO_ID    (11)        // 高锰酸钾 ID
#define PARAM_SENSOR4_AN_DAN_ID (12)        // 氨氮 ID
#define PARAM_SENSOR4_LIN_ID    (13)        // 磷 ID
#define PARAM_SENSOR4_AN_ID     (14)        // 氨 ID

typedef struct {
	uint8_t success;
	float ph_value;
	float cond_value;
	float do_value;
	float turb_value;
	float temp_value;
}ParamSensor5;

typedef struct {
	uint8_t success;
	float gao_value;
	float andan_value;
	float lin_value;
	float an_value;
}ParamSensor4;

void param_sensor_init(void);

void param_sensor_restart(void);

void param_sensor_update(void);

const ParamSensor5* get_current_param_sensor5(void);

const ParamSensor4* get_current_param_sensor4(void);

#endif
