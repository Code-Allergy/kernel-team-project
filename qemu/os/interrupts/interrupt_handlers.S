#define INTC_BASE_ADDR              0x01C20400
#define INTC_IRQ_PENDING_OFF        0x10

#define MODE_Usr 		0x10	/* thread-mode, unprivileged */
#define MODE_IRQ 		0x12	/* IRQ-mode (always privileged) */
#define MODE_Supervisor 0x13	/* SVC-mode (always privileged) */
#define MODE_Abort 		0x17	/* Abort-mode (always privileged) */
#define MODE_Undef	 	0x1B	/* Undefined-mode (always privileged) */
#define MODE_System 	0x1F	/* thread-mode, privileged */
#define I_BIT           0x80	/* I bit for CPSR register */


.global reset_handler_asm
.global undef_handler_asm
.global svc_handler_asm
.global prefetch_abort_handler_asm
.global data_abort_handler_asm
.global irq_handler_asm
.global fiq_handler_asm
.global reserved_handler_asm

.extern intc_vector_table

.section .text

reset_handler_asm:
    .word __init
    b .

_irq_handler_asm:
    sub      lr, lr, #4             /* Adjust return address */
    stmfd    sp!, {r0-r3, r12, lr}  /* Save registers */
    mrs      r12, spsr              /* Save spsr */
    stmfd    sp!, {r12}             /* Push spsr */

    /* Get pending IRQs and find active */
    ldr      r1, =(INTC_BASE_ADDR + INTC_IRQ_PENDING_OFF)
    ldr      r2, [r1]               /* Load pending bits */
    clz      r3, r2                 /* Count leading zeros */
    rsb      r2, r3, #31            /* IRQ number = 31 - clz */

    /* Switch to System mode and enable IRQs */
    mrs      lr, cpsr
    orr      lr, lr, #MODE_System
    bic      lr, lr, #I_BIT
    msr      cpsr_c, lr

    stmfd    sp!, {lr}              /* Save lr_usr */

    /* Jump to ISR via vector table */
    ldr      r0, =intc_vector_table
    ldr      pc, [r0, r2, lsl #2]

    /* Restore context */
    ldmfd    sp!, {lr}              /* Restore lr_usr */
    cpsid    i, #MODE_IRQ           /* Back to IRQ mode, disable IRQs */
    ldmfd    sp!, {r12}             /* Restore spsr */
    msr      spsr_cxsf, r12
    ldmfd    sp!, {r0-r3, r12, pc}^ /* Restore regs and return */


irq_handler_asm:
    STMDB   SP!, {R0-R12, LR}             @ Save kernel registers
    MRS     R0, SPSR                      @ Get SPSR_irq
    STMDB   SP!, {R0}                     @ Save SPSR onto the stack


    MOV    R0, #0                         @ load 0 to R0 to indicate kernel mode
    BL      handle_irq_c                  @ Call IRQ handler

    @ Restore registers
    LDMIA   SP!, {R0}                     @ Restore SPSR from stack
    MSR     SPSR_cxsf, R0                 @ Restore SPSR
    LDMIA   SP!, {R0-R12, LR}              @ Restore registers

    SUBS    PC, LR, #4                     @ Return to kernel (adjust IRQ offset)

/* Unimplemented handlers */
undef_handler_asm:
svc_handler_asm:
prefetch_abort_handler_asm:
data_abort_handler_asm:
fiq_handler_asm:
reserved_handler_asm:
    b .

/**
* @brief Set the vector table base address
* 
* @param r0 - Address of the vector table
*/
.global set_vector_table_base_addr_asm
set_vector_table_base_addr_asm:
    mcr     p15, #0, r0, c12, c0, #0
    dsb
    bx      lr