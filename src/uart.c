#include <memory_map.h>
#include <uart.h>

static volatile unsigned int* uart_dr = (unsigned int*)(UART0_DR);

void uart_init(){
    return;
}

void uart_putc(char c){
    *uart_dr = (unsigned int)c;
    return;
}

void uart_puts(const char *s){
    while(*s != '\0'){
        uart_putc(*s);
        s++;
    }
    return;
}