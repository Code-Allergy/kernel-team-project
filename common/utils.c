#include <utils.h>


void check_mode(void) {
    unsigned int cpsr;
    __asm__ volatile ("mrs %0, cpsr" : "=r"(cpsr));

    unsigned int mode = cpsr & 0x1F;
    uart_printf("CPSR: 0x%x (Mode bits: 0x%x) — ", cpsr, mode);

    switch (mode) {
        case 0x10: uart_puts("User Mode\n"); break;
        case 0x11: uart_puts("FIQ Mode\n"); break;
        case 0x12: uart_puts("IRQ Mode\n"); break;
        case 0x13: uart_puts("Supervisor Mode\n"); break;
        case 0x17: uart_puts("Abort Mode\n"); break;
        case 0x1B: uart_puts("Undefined Mode\n"); break;
        case 0x1F: uart_puts("System Mode\n"); break;
        default:   uart_puts("Unknown Mode\n"); break;
    }
}

void foo(){


}
