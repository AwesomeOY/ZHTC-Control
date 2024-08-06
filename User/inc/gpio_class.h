#ifndef GPIO_CLASS_H_
#define GPIO_CLASS_H_

#include "userdef.h"

typedef enum {
	GPIO_TYPE_READ = 0,
	GPIO_TYPE_WRITE,
}GPIO_TYPE;

typedef enum {
	GPIO_PIN_LOW = GPIO_PIN_RESET,
	GPIO_PIN_HIGH = GPIO_PIN_SET
}GPIO_LEVEL;

typedef struct {
	GPIO_TYPE type;
	GPIO_LEVEL valid_level;
	Pin pin;
}gpio_obj;

void gpio_output_valid(const gpio_obj* gpio);

void gpio_output_invalid(const gpio_obj* gpio);

unsigned char gpio_input_valid(const gpio_obj* gpio);

#endif
