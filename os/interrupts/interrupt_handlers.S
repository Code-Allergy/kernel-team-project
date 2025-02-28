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

.extern intc_vector_table

.section .text

reset_handler_asm:
    .word __init
    B .

irq_handler_asm:
    SUB      r14, r14, #4             // Apply lr correction
    STMFD    r13!, {r0-r3, r12, r14}  // Save context
    MRS      r12, spsr                // Copy spsr
    STMFD    r13!, {r12}

    LDR      r0, =(INTC_BASE_ADDR + INTC_THRESHOLD_OFF)         // Get the IRQ Threshold
    LDR      r1, [r0, #0]             
    STMFD    r13!, {r1}                                         // Save the threshold value
    
    LDR      r2, =(INTC_BASE_ADDR + INTC_IRQ_PRIORITY_OFF)      // Get the IRQ Priority
    LDR      r3, [r2, #0]
    STR      r3, [r0, #0]                                       // Set the priority as threshold

    LDR      r1, =(INTC_BASE_ADDR + INTC_SIR_IRQ_OFF)           // Get the Active IRQ
    LDR      r2, [r1]
    AND      r2, r2, #0x7F                                      // Mask the Active IRQ number

    MOV      r0, #0x01                                          // To enable new IRQ Generation
    LDR      r1, =(INTC_BASE_ADDR + INTC_CONTROL_OFF)           // Get the Control Register


    CMP      r3, #0                   // Check if non-maskable priority 0
    STRNE    r0, [r1]                 // if > 0 priority, acknowledge INTC
    DSB                               // Make sure acknowledgement is completed

    //
    // Enable IRQ and switch to system mode. But IRQ shall be enabled
    // only if priority level is > 0. Note that priority 0 is non maskable.
    // Interrupt Service Routines will execute in System Mode.
    //
    MRS      r14, cpsr                // Read cpsr
    ORR      r14, r14, #MODE_System   // Switch to System Mode
    BICNE    r14, r14, #I_BIT         // Enable IRQ if priority > 0
    MSR      cpsr, r14
                            
    
    STMFD    r13!, {r14}              // Save lr_usr

    LDR      r0, =intc_vector_table   // Load the base of the vector table
    ADD      r14, pc, #0              // Save return address in LR
    LDR      pc, [r0, r2, lsl #2]     // Jump to the ISR

    LDMFD    r13!, {r14}              // Restore lr_usr

    //
    // Disable IRQ and change back to IRQ mode
    //
    CPSID    i, #MODE_IRQ             // Disable IRQ

    LDR      r0, =(INTC_BASE_ADDR + INTC_THRESHOLD_OFF)      // Get the IRQ Threshold

    LDR      r1, [r0, #0]
    CMP      r1, #0                   // If priority 0
    MOVEQ    r2, #0x01           // Enable new IRQ Generation
    LDREQ    r1, =(INTC_BASE_ADDR + INTC_CONTROL_OFF) 
    STREQ    r2, [r1]
    
    LDMFD    r13!, {r1}
    STR      r1, [r0, #0]             // Restore the threshold value
    LDMFD    r13!, {r12}
    MSR      spsr, r12                // Restore spsr

    LDMFD    r13!, {r0-r3, r12, pc}^  // Restore the context and return



// Unimplemented handlers
undef_handler_asm:
svc_handler_asm:
prefetch_abort_handler_asm:
data_abort_handler_asm:
fiq_handler_asm:
reserved_handler_asm:
    B .

/**
* @brief Set the vector table base address
* 
* @param r0 - Address of the vector table
*/
.global set_vector_table_base_addr_asm
set_vector_table_base_addr_asm:
    MCR     p15, #0, r0, c12, c0, #0
    DSB
    BX      lr