#include "level_switch.h"

void level_switch_input_init(level_switch_input* pls, Pin input_pin, uint8_t level)
{
	pls->pin = input_pin;
	pls->valid_level = level;
}

LEVEL_SWITCH_STATUS level_switch_input_read(level_switch_input* pls)
{
	if (READ_PIN(pls->pin.port, pls->pin.pin) == pls->valid_level) {
		return LEVEL_SWITCH_STATUS_OPEN;
	}
	return LEVEL_SWITCH_STATUS_CLOSE;
}

