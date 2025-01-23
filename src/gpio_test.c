#include <gpio.h>
#include <uart.h>

#define LED_PINS (0xF << 21)

void delay(volatile unsigned int count) {
    while (count--);
}

void dumb_delay() {
    volatile unsigned int count = 0x3FFFFFF;
    while (count--);
}


void gpio_test() {
    unsigned int gpio_base = GPIO1_BASE;

    GPIO_init(); // Currently only configures GPIO1

    uart_init();


    uart_puts("GPIO test\n");


    while (1) {
        GPIO_set(gpio_base, LED_PINS);
        delay(0x1FFFFFF);
        GPIO_clear(gpio_base, LED_PINS);
        delay(0x1FFFFFF);
    }
}