#include <gpio.h>
#include <uart.h>

#define LED_PINS (0xF << 21)

static inline void delay(volatile unsigned int count)
{
    while (count--)
        ;
}

static inline void dumb_delay()
{
    volatile unsigned int count = 0x3FFFFFF;
    while (count--)
        ;
}

static inline void gpio_test()
{
    unsigned int gpio_base = GPIO1_BASE;

    delay(0xFFFF);
    GPIO_init(); /*  Currently only configures GPIO1 */
    GPIO_set(GPIO1_BASE, 1 << 21);

    while (1)
    {
        GPIO_set(gpio_base, LED_PINS);
        /* uart_puts("LEDs on!\n"); */
        delay(0x1FFFFFF);
        GPIO_clear(gpio_base, LED_PINS);
        /* uart_puts("LEDs off!\n"); */
        delay(0x1FFFFFF);
    }
}
