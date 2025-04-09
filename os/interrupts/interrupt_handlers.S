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
.extern os_sp_cache
.extern usr_sp_cache
.extern irq_lr_cache
.extern save_context_asm
.extern save_sp_and_lr_usr
.extern load_os_sp


.section .text

reset_handler_asm:
    /* .word __init */
    b .

irq_handler_asm:
    sub      r14, r14, #4           /* Apply lr correction */
    @str      r13, usr_sp_cache
    @ldr      r13, os_sp_cache       /* Get the os stack pointer */
    stmfd    r13!, {r0-r12, r14}    /* Save context */
    mrs      r12, spsr              /* Copy spsr */
    stmfd    r13!, {r12}

    /* bl   save_context (save to pcb and return) */

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

    /* Enable new IRQ generation if priority is > 0 (maskable)*/
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

    /* b  scheduler  (should not return) */

    /* These will never be reached once scheduler is implemented */
    ldmfd    r13!, {r12}
    msr      spsr, r12                /* Restore spsr */
    ldmfd    r13!, {r0-r12, pc}^  /* Restore the context and return */


svc_handler_asm:
    @  save r0 and r1 so that we can use them
    stmfd    sp!, {r0}                /* Push r0 to the stack */
    stmfd    sp!, {r1}                /* Push r1 to the stack */
    
    @switch to system mode (same cpu registers as uer mode)
    mrs      r1, cpsr                /* Read cpsr */
    orr      r1, r1, #MODE_System   /* Switch to System Mode */
    msr      cpsr, r1

    @ cache sp and lr for scheduler ctx saving
    mov       r0, r13
    mov       r1, r14
    bl        save_sp_and_lr_usr

    @ back to SVC mode
    cpsid    i, #MODE_Supervisor     /* Disable IRQ */

    @ restore r0 and r1
    ldmfd    sp!, {r1}                /* Pop r1 from the stack */
    ldmfd    sp!, {r0}                /* Pop r0 from the stack */

    @ back to system mode
    msr cpsr, #MODE_System

    bl       load_os_sp               /* Load the os stack pointer */
    bl       save_context_asm         /* Save context to PCB */

    @ call  scheduler_run
    @ scheduler_run should not return
    b        scheduler_run

    b        loop_forever


/*
 * syscall:
 * This function triggers a software interrupt (syscall) and returns to the caller.
 * Takes variable args, first argument (r0) is the syscall number
 * and the remaining arguments are the arguments needed by the
 * syscall handler
 * 
 * Arguments:
 * - r0: The syscall number.
 * - r1, r2, r3, etc.: The arguments needed by the syscall handler.
 * 
 * Returns:
 * - The result of the syscall is stored in r0.
 */
syscall:
    swi #0       /* Trigger software interrupt (syscall) */
    
    bx lr        /* Return to caller (R0 holds syscall result) */



/*
.global save_context
save_context:
    ldr r0, =current_process
    ldr r0, [r0]             @ r0 = process_t*
    add r1, r0, #12          @ r1 = &proc->registers[0]

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
*/


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
