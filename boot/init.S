.arm

/* these are not real */
#define MODE_Usr 		0x10	/* thread-mode, unprivileged */
#define MODE_FIQ 		0x11	/* FIQ-mode (always privileged) */
#define MODE_IRQ 		0x12	/* IRQ-mode (always privileged) */
#define MODE_Supervisor 0x13	/* SVC-mode (always privileged) */
#define MODE_Abort 		0x17	/* Abort-mode (always privileged) */
#define MODE_Undef	 	0x1B	/* Undefined-mode (always privileged) */
#define MODE_System 	0x1F	/* thread-mode, privileged */
#define I_F_BIT  		0xC0	/* I and F bits for CPSR register */

#define STACK_SIZE 0x1000 /* 4KB */

.global __init
__init:

	/* https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/cps */
	/* cps (change processor state) id (interrupt / abort disable) */
	/* cpsid aif, #MODE_System	/* disable : imprecise aborts, irq interrupt, fiq interrupt. Enter mode #MODE_System */

	/* Set up stacks for different CPU modes */
	/* Enter IRQ mode */
	ldr r0, =__StackStart
	msr cpsr_c, #(MODE_IRQ|I_F_BIT)			
	mov sp, r0
	sub r0, r0, #STACK_SIZE

	/* Enter FIQ mode */
	msr cpsr_c, #(MODE_FIQ|I_F_BIT)			
	mov sp, r0
	sub r0, r0, #STACK_SIZE

	/* Enter Abort mode */
	msr cpsr_c, #(MODE_Abort|I_F_BIT)		
	mov sp, r0
	sub r0, r0, #STACK_SIZE

	/* Enter Undefined mode */
	msr cpsr_c, #(MODE_Undef|I_F_BIT)		
	mov sp, r0
	sub r0, r0, #STACK_SIZE
	
	/* Enter Supervisor mode */
	msr cpsr_c, #(MODE_Supervisor|I_F_BIT)	
	mov sp, r0
	sub r0, r0, #STACK_SIZE
	
	/* Enter System mode */
	msr cpsr_c, #(MODE_System|I_F_BIT)		
	mov sp, r0


	/* cortex v8, page 3-45 - cache and branch predictor */
	mrc	p15, 0, r0, c1, c0, 0	/* read system control register */
	/* data cache, strict alignment, MMU (MMU = 1 current breaks, idk) */
	orr	r0, r0, #0b100
	/* Instruction cache, Program flow predict */
	orr r0, r0, #(0b11 << 11)
	/* enable arm exceptions, clear the 30th bit in r0 */
	bic r0, r0, #(1 << 30)
	mcr	p15, 0, r0, c1, c0, 0	/* system control register */

	/* L2 cache enable */
	mrc p15, 0, r0, c1, c0, 1
	orr r0, r0, #0b10
	mcr p15, 0, r0, c1, c0, 1 /* auxiliary control reg */
	

	/* cpsie	aif			/* reenable all interrupts */
	
	bl	__BootloaderEntry

idle:
	wfi 		/* wait for interrupt */
	b	idle