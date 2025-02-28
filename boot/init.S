.arm

// these are not real
#define MODE_Usr 		0x10	// thread-mode, unprivileged
#define MODE_FIQ 		0x11	// FIQ-mode (always privileged)
#define MODE_IRQ 		0x12	// IRQ-mode (always privileged)
#define MODE_Supervisor 0x13	// SVC-mode (always privileged)
#define MODE_Abort 		0x17	// Abort-mode (always privileged)
#define MODE_Undef	 	0x1B	// Undefined-mode (always privileged)
#define MODE_System 	0x1F	// thread-mode, privileged
#define I_F_BIT  		0xC0	// I and F bits for CPSR register

#define STACK_SIZE 0x1000

.global __init
__init:

	// https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/cps
	// cps (change processor state) id (interrupt / abort disable)
	// cpsid aif, #MODE_System	// disable : imprecise aborts, irq interrupt, fiq interrupt. Enter mode #MODE_System

	// Set up stacks for different CPU modes
    ldr r0, =__StackStart
    msr cpsr_c, #(MODE_IRQ|I_F_BIT)			// Enter IRQ mode
    mov sp, r0
	sub r0, r0, #STACK_SIZE

    msr cpsr_c, #(MODE_FIQ|I_F_BIT)			// Enter FIQ mode
	mov sp, r0
	sub r0, r0, #STACK_SIZE

    msr cpsr_c, #(MODE_Abort|I_F_BIT)		// Enter Abort mode
	mov sp, r0
	sub r0, r0, #STACK_SIZE

	msr cpsr_c, #(MODE_Undef|I_F_BIT)		// Enter Undefined mode
	mov sp, r0
	sub r0, r0, #STACK_SIZE
	
	msr cpsr_c, #(MODE_Supervisor|I_F_BIT)	// Enter Supervisor mode
	mov sp, r0
	sub r0, r0, #STACK_SIZE

	msr cpsr_c, #(MODE_System|I_F_BIT)		// Enter System mode
	mov sp, r0


    // // Set Vector Table Base Address
    // ldr r0, =asm_vector_table  
    // mcr p15, 0, r0, c12, c0, 0

	// cortex v8, page 3-45 - cache and branch predictor
	mrc	p15, 0, r0, c1, c0, 0	// read system control register
	// data cache, strict alignment, MMU (MMU = 1 current breaks, idk)
	orr	r0, r0, #0b100
	// Instruction cache, Program flow predict 
	orr r0, r0, #(0b11 << 11)
	// enable arm exceptions, clear the 30th bit in r0
	bic r0, r0, #(1 << 30)
	mcr	p15, 0, r0, c1, c0, 0	// system control register

	// L2 cache enable
	mrc p15, 0, r0, c1, c0, 1
	orr r0, r0, #0b10
	mcr p15, 0, r0, c1, c0, 1 // auxiliary control reg
	
	// Initialize the interrupt controller
	//bl intc_init

	//cpsie	aif			// reenable all interrupts
	
	bl	__BootloaderEntry

idle:
	wfi 		// wait for interrupt
	b	idle


# 6.2.1 Initialization Sequence
# 1. Program the MPU_INTC.INTC_SYSCONFIG register: If necessary, enable the interface clock
# autogating by setting the AUTOIDLE bit.
# 2. Program the MPU_INTC.INTC_IDLE register: If necessary, disable functional clock autogating or
# enable synchronizer autogating by setting the FUNCIDLE bit or TURBO bit accordingly.
# 3. Program the MPU_INTC.INTC_ILRm register for each interrupt line: Assign a priority level and set the
# FIQNIRQ bit for an FIQ interrupt (by default, interrupts are mapped to IRQ and priority is 0x0 [highest]).
# 4. Program the MPU_INTC.INTC_MIRn register: Enable interrupts (by default, all interrupt lines are
# masked). NOTE: To program the MPU_INTC.INTC_MIRn register, the MPU_INTC.INTC_MIR_SETn
# and MPU_INTC.INTC_MIR_CLEARn registers are provided to facilitate the masking, even if it is
# possible for backward-compatibility to write directly to the MPU_INTC.INTC_MIRn register.
// intc_init:
// 	# INTC_SYSCONFIG soft reset
// 	ldr r0, =0x48200010
// 	ldr r1, [r0]
// 	orr r1, r1, #0x2
// 	str r1, [r0]
// 	# wait for reset to complete
// 	ldr r1, [r0]
// 	ldr r1, [r0]
// 	# INTC_SYSCONFIG disable auto idle
// 	ldr r1, =0x0
// 	str r1, [r0]
// 	# INTC_IDLE disable functional clock autogating
// 	ldr r0, =0x48200050
// 	ldr r1, =0x1
// 	str r1, [r0]
// 	# Unmask UART0 interrupt in INTC_MIRn
// 	# INTC_MIR0 -> Iterrupts 0-31
// 	# INTC_MIR1 -> Iterrupts 32-63
// 	# INTC_MIR2 -> Iterrupts 64-95
// 	# INTC_MIR3 -> Iterrupts 96-127
// 	# Uart0 interrupt is 72 so using INTC_MIR2, bit 8
// 	ldr r0, =0x482000C8  /* INTC_MIR_CLEAR2 */
// 	ldr r1, [r0]
// 	orr r1, r1, #(0x1<<8)
// 	str r1, [r0]
// 
// 	bx lr
// 
// undef_handler_asm:
// 	b undef_handler_asm
// 
// svc_handler_asm:
// 	b svc_handler_asm
// 
// prefetch_abort_handler_asm:
// 	b prefetch_abort_handler_asm
// 
// data_abort_handler_asm:
// 	b data_abort_handler_asm
// 
// irq_handler_asm:
// 	# turn on LED0
// 	ldr r0, =(0x4804c000 + 0x194)
// 	ldr r1, [r0]
// 	orr r1, r1, #(0x1 << 21)
// 	str r1, [r0]
// 
// 	sub lr, lr, #4
// 	stmfd sp!, {r0-r12, lr}
// 	mrs r0, spsr
// 	stmfd sp!, {r0}
// 	bl irq_handler
// 	ldmfd sp!, {r0}
// 	msr SPSR_cxsf, r0
// 	ldmfd sp!, {r0-r12, lr}
// 	subs pc, lr, #0
// 
// fiq_handler_asm:
// 	b fiq_handler_asm
// 
// 
// .section .asm_vectors, "ax"
// .global asm_vector_table
// asm_vector_table:
// 	.word __init
// 	.word undef_handler_asm
// 	.word svc_handler_asm
// 	.word prefetch_abort_handler_asm
// 	.word data_abort_handler_asm
// 	.word .
// 	.word irq_handler_asm
// 	.word fiq_handler_asm
// 
// # .global setup_vbar
// # setup_vbar:
// #     ldr r0, =vector_table    @ Load vector table address
// #     mcr p15, 0, r0, c12, c0, 0  @ Write to VBAR register
// #     bx lr
