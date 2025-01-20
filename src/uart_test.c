#include <uart.h>

void uart_test(){
    uart_init();
    uart_puts("Hello, World!\n");
    return;
}