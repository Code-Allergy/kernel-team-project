#include <memory_map.h>
#include "uart.h"
#include "gpio.h"


int leds = 1;
void _uart_init( unsigned short  uart,
                unsigned int    baud_rate, 
                unsigned short  stop_bit_en,
                unsigned short  parity_en, 
                unsigned short  parity_type, 
                unsigned short  char_length
            ) {
    // Only support UART0 for now
    switch (uart){
        case 0:


            break;
        default:
            break;
    }

}


void uart_init(void) {
    GPIO_set(GPIO1_BASE, leds<<21);
    _uart_init(0, 115200, 0, 0, 0, 8);
}

void uart_putc(char c) {
//     while (!(UART0_REG32(UART_LSR_UART_OFF) & 0x20));  // Wait for the THR empty bit to be set
//     UART0_REG32(UART_THR_OFF) = c;  // Write the character to the THR
}

void uart_puts(const char *str) {
    while (*str) {
        uart_putc(*str++);
    }
}