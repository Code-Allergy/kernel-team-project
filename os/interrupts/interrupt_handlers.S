#define INTC_BASE_ADDR              0x48200000
#define INTC_CONTROL_OFF            0x48
#define INTC_SIR_IRQ_OFF            0x40
#define INTC_SYSCONFIG_OFF          0x10
#define INTC_IDLE_OFF               0x50
#define INTC_THRESHOLD_OFF          0x68
#define INTC_STATUS_OFF             0x14
#define INTC_IRQ_PRIORITY_OFF       0x60

#define MODE_Usr 		0x10	/* thread-mode, unprivileged */
#define MODE_FIQ 		0x11	/* FIQ-mode (always privileged) */
#define MODE_IRQ 		0x12	/* IRQ-mode (always privileged) */
#define MODE_Supervisor 0x13	/* SVC-mode (always privileged) */
#define MODE_Abort 		0x17	/* Abort-mode (always privileged) */
#define MODE_Undef	 	0x1B	/* Undefined-mode (always privileged) */
#define MODE_System 	0x1F	/* thread-mode, privileged */
#define I_F_BIT  		0xC0	/* I and F bits for CPSR register */
#define I_BIT           0x80	/* I bit for CPSR register */


.global reset_handler_asm
.global undef_handler_asm
.global svc_handler_asm
.global prefetch_abort_handler_asm
.global data_abort_handler_asm
.global irq_handler_asm
.global fiq_handler_asm
.global reserved_handler_asm
.global syscall

.extern intc_vector_table
.extern undef_handler
.extern prefetch_abort_handler
.extern data_abort_handler
.extern scheduler_run

.global mmu_undef_handler_asm
.extern dump_undef_info
.extern irq_handler
.extern current_process

.section .text

reset_handler_asm:
    /* .word __init */
    b .

irq_handler_asm:
    sub      lr, lr, #4               @ Adjust return address (actual PC)
    stmfd    sp!, {lr}                @ Save return address (PC)
    stmfd    sp!, {r0-r12}            @ Save general-purpose registers
    mrs      r12, spsr                @ Save user CPSR
    bic      r12, r12, #(1 << 7)      @ Enable IRQs again in saved CPSR
    stmfd    sp!, {r12}               @ Save SPSR 
    
    @bl irq_handler
    bl   save_context /* (save to pcb and return) */

    /* Save the threshold value */
    ldr      r0, =(INTC_BASE_ADDR + INTC_THRESHOLD_OFF)
    ldr      r1, [r0, #0]
    stmfd    r13!, {r1}

    /* Set the priority as threshold.
    interrupt with priority less than threshold will not be serviced */
    ldr      r2, =(INTC_BASE_ADDR + INTC_IRQ_PRIORITY_OFF)
    ldr      r3, [r2, #0]
    str      r3, [r0, #0]

    /* Get the active IRQ */
    ldr      r1, =(INTC_BASE_ADDR + INTC_SIR_IRQ_OFF)
    ldr      r2, [r1]
    and      r2, r2, #0x7f

    /* Enable new IRQ generation if priority is > (maskable)*/
    mov      r0, #0x01
    ldr      r1, =(INTC_BASE_ADDR + INTC_CONTROL_OFF)
    cmp      r3, #0
    strne    r0, [r1]                 /* if > 0 priority, acknowledge INTC */
    dsb                               /* Make sure acknowledgement is completed */


    mrs      r14, cpsr                /* Read cpsr */
    orr      r14, r14, #MODE_System   /* Switch to System Mode */
    bicne    r14, r14, #I_BIT         /* Enable IRQ if priority > 0 */
    msr      cpsr, r14

    stmfd    r13!, {r14}              /* Save lr_usr */

    ldr      r0, =intc_vector_table   /* Load the base of the vector table */
    add      r14, pc, #0              /* Save return address in lr */
    ldr      pc, [r0, r2, lsl #2]     /* Jump to the ISR. pc = vector_base + (intc_num * 4) */

    /* Interrupt has been serviced. Restore the context */
    ldmfd    r13!, {r14}              /* Restore lr_usr */

    /* Disable IRQ and change back to IRQ mode */
    cpsid    i, #MODE_IRQ             /* Disable IRQ */

    ldr      r0, =(INTC_BASE_ADDR + INTC_THRESHOLD_OFF)      /* Get the IRQ Threshold */
    ldr      r1, [r0, #0]
    /* If the threshold is 0, then enable new IRQ generation
    (was disabled in the beginning) */
    cmp      r1, #0                   /* If priority 0 */
    moveq    r2, #0x01                /* Enable new IRQ Generation */
    ldreq    r1, =(INTC_BASE_ADDR + INTC_CONTROL_OFF)
    streq    r2, [r1]

    ldmfd    r13!, {r1}
    str      r1, [r0, #0]             /* Restore the threshold value */

  @ Switch to System mode to read user SP
    mrs     r0, cpsr              @ r0 = current CPSR (in IRQ mode)
    bic     r1, r0, #0x10         @ clear mode bits
    orr     r1, r1, #0x10         @ set to User mode (0x1F)
    msr     cpsr_c, r1            
    nop

    mrs     r3, cpsr              @ confirm mode switch
    and     r3, r3, #0x10
    cmp     r3, #0x10
    bne     1f                    @ if not System mode, go into infinite loop
    
    

    mov     r2, sp                @ r2 = SP_usr

    swi 102 

    b  scheduler_run


1:
    b       1b                    @ infinite loop if mode switch failed


.global save_context
save_context:
    ldr r0, =current_process
    ldr r0, [r0]             @ r0 = process_t*
    add r1, r0, #12          @ r1 = &proc->registers[0]

    ldmfd sp!, {r2}          @ Pop user SP
    str   r2, [r0, #64]      @ SP_OFFSET

    ldmfd sp!, {r2}          @ Pop SPSR
    str   r2, [r0, #72]      @ CPSR_OFFSET

    @ Pop r0–r12 in reverse order
    ldmfd sp!, {r2}          @ r12
    str   r2, [r1, #48]
    ldmfd sp!, {r2}          @ r11
    str   r2, [r1, #44]
    ldmfd sp!, {r2}          @ r10
    str   r2, [r1, #40]
    ldmfd sp!, {r2}          @ r9
    str   r2, [r1, #36]
    ldmfd sp!, {r2}          @ r8
    str   r2, [r1, #32]
    ldmfd sp!, {r2}          @ r7
    str   r2, [r1, #28]
    ldmfd sp!, {r2}          @ r6
    str   r2, [r1, #24]
    ldmfd sp!, {r2}          @ r5
    str   r2, [r1, #20]
    ldmfd sp!, {r2}          @ r4
    str   r2, [r1, #16]
    ldmfd sp!, {r2}          @ r3
    str   r2, [r1, #12]
    ldmfd sp!, {r2}          @ r2
    str   r2, [r1, #8]
    ldmfd sp!, {r2}          @ r1
    str   r2, [r1, #4]
    ldmfd sp!, {r2}          @ r0
    str   r2, [r1, #0]

    ldmfd sp!, {r2}          @ Pop saved PC
    str   r2, [r0, #76]      @ PC_OFFSET

    bx lr



mmu_undef_handler_asm:
    sub lr, lr, #4              @ LR = faulting instruction
    mrs r0, cpsr                @ r0 = CPSR
    mov r1, lr                  @ r1 = fault address
    bl dump_undef_info          @ call C function with r0 (cpsr) and r1 (addr)

1:
    b 1b                        @ infinite loop



loop_forever:
    b loop_forever 


/* Unimplemented handlers */
undef_handler_asm:
    b undef_handler
prefetch_abort_handler_asm:
    b prefetch_abort_handler
data_abort_handler_asm:
    b data_abort_handler
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
