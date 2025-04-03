.section .text
.global get_irq_sp

get_irq_sp:
    mrs r0, cpsr              @ Save current mode
    orr r1, r0, #0x12         @ Switch to IRQ mode
    msr cpsr_c, r1            @ Set IRQ mode
    mov r0, sp                @ Load IRQ stack pointer into r0
    msr cpsr_c, r0            @ Restore original mode
    bx lr                     @ Return to caller
