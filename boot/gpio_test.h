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
    GPIO_init(); // Currently only configures GPIO1
    GPIO_set(GPIO1_BASE, 1 << 21);
    // 8N1
    /* uart_init( */
    /*     0,         // UART index (0 = UART0, 1 = UART1, etc.) */
    /*     115200,    // Baud rate for communication */
    /*     1,         // Stop bit enable (1 = enabled, 0 = disabled) */
    /*     0,         // Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits) */
    /*     0,         // Parity enable (1 = enabled, 0 = disabled) */
    /*     0,         // Parity type (0 = even, 1 = odd; ignored if parity is disabled) */
    /*     8          // Character length */
    /* ); */

    /* uart_puts("Sup bro\n"); */

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
