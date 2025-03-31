.global save_registers
save_registers:
    @ r0: pointer to registers[0] in PCB

    pop {r1-r12}                 @ Pop r1–r12
    str r1, [r0, #0]
    str r2, [r0, #4]
    str r3, [r0, #8]
    str r4, [r0, #12]
    str r5, [r0, #16]
    str r6, [r0, #20]
    str r7, [r0, #24]
    str r8, [r0, #28]
    str r9, [r0, #32]
    str r10, [r0, #36]
    str r11, [r0, #40]
    str r12, [r0, #44]

    pop {r1}                     @ sp
    str r1, [r0, #52]

    pop {r2}                     @ lr
    str r2, [r0, #56]

    mrs r3, cpsr
    str r3, [r0, #60]            @ cpsr

    bx lr

.global load_registers
load_registers:
    @ r0: pointer to PCB (process_t*)

    ldr r1, [r0, #72]           @ Load CPSR (offset of cpsr)
    msr cpsr, r1                @ Restore CPSR

    ldr sp, [r0, #64]           @ Load saved SP

    pop {r0-r12, lr, pc}        @ Restore registers and jump to saved PC



.section .text
.global get_irq_sp

get_irq_sp:
    mrs r0, cpsr              @ Save current mode
    orr r1, r0, #0x12         @ Switch to IRQ mode
    msr cpsr_c, r1            @ Set IRQ mode
    mov r0, sp                @ Load IRQ stack pointer into r0
    msr cpsr_c, r0            @ Restore original mode
    bx lr                     @ Return to caller
