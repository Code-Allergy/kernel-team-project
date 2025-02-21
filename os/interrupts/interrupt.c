#include <interrupt.h>
#include <syscall.h>
#include <types.h>

/* Read a hypothetical IRQ status register */
#define IRQ_STATUS_REG  (*(volatile uint32_t*)0x48200000)

/* IRQ Handler */
void irq_handler(void) {
    uint32_t irq_status = IRQ_STATUS_REG;

    if (irq_status & (1 << 5)) {
        handle_uart_interrupt();
    }
    if (irq_status & (1 << 10)) {
        handle_gpio_interrupt();
    }

    /* Acknowledge interrupt */
    IRQ_STATUS_REG = irq_status;
}

/* SWI Handler */
void svc_handler(void) {
    uint32_t svc_number;
    
    __asm__("MRS %0, SPSR" : "=r" (svc_number));

    handle_syscall(svc_number);
}
