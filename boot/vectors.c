#include "vectors.h"

/* Function prototypes */
void reset_handler(void);
void undef_handler(void);
void swi_handler(void);
void prefetch_abort_handler(void);
void data_abort_handler(void);
void irq_handler(void);
void fiq_handler(void);

/* Weak default handlers - can be overridden */
__attribute__((weak)) void reset_handler(void) { while (1); }
__attribute__((weak)) void undef_handler(void) { while (1); }
__attribute__((weak)) void swi_handler(void) { while (1); }
__attribute__((weak)) void prefetch_abort_handler(void) { while (1); }
__attribute__((weak)) void data_abort_handler(void) { while (1); }
__attribute__((weak)) void irq_handler(void) { while (1); }
__attribute__((weak)) void fiq_handler(void) { while (1); }

__attribute__((section(".vectors")))
void (*const vector_table[])(void) = {
    reset_handler,          // Reset
    undefined_handler,      // Undefined Instruction
    svc_handler,            // Supervisor Call (SVC)
    prefetch_abort_handler, // Prefetch Abort
    data_abort_handler,     // Data Abort
    0,                      // Reserved
    irq_handler,            // IRQ (Interrupt Request)
    fiq_handler             // FIQ (Fast Interrupt Request)
};