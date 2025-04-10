#include <gpio.h>

void GPIO_init(void) { /* dummy, we can emulate the GPIOs or log the calls */ }

void GpioSetPinMode(const enum GpioIOBase Gpio,
                    const unsigned int PinMask,
                    const enum GpioPinDirection Dir)
{
    /* dummy, we can emulate the GPIOs or log the calls */
}

void GPIO_set(unsigned int gpio_base, unsigned int pins)
{
    /* dummy, we can emulate the GPIOs or log the calls */
}

void GPIO_clear(unsigned int gpio_base, unsigned int pins)
{
    /* dummy, we can emulate the GPIOs or log the calls */
}