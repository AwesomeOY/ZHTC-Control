#include "gpio_class.h"

void gpio_output_valid(const gpio_obj* gpio)
{
	WRITE_PIN(gpio->pin.port, gpio->pin.pin, gpio->valid_level);
}

void gpio_output_invalid(const gpio_obj* gpio)
{
	GPIO_LEVEL level = (gpio->valid_level == GPIO_PIN_HIGH) ? GPIO_PIN_LOW : GPIO_PIN_HIGH;
	WRITE_PIN(gpio->pin.port, gpio->pin.pin, level);
}

unsigned char gpio_input_valid(const gpio_obj* gpio)
{
	return ((GPIO_LEVEL)READ_PIN(gpio->pin.port, gpio->pin.pin)) == gpio->valid_level;
}
