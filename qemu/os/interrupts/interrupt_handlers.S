#define INTC_BASE_ADDR              0x01C20400
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

.extern intc_vector_table

.section .text

reset_handler_asm:
    .word __init
    b .

irq_handler_asm:
    sub      r14, r14, #4             /* Apply lr correction */
    stmfd    r13!, {r0-r3, r12, r14}  /* Save context */
    mrs      r12, spsr                /* Copy spsr */
    stmfd    r13!, {r12}

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
    ldmfd    r13!, {r12}
    msr      spsr, r12                /* Restore spsr */

    ldmfd    r13!, {r0-r3, r12, pc}^  /* Restore the context and return */



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