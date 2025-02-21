.arm

// these are not real
#define MODE_Usr 0b10000	// thread-mode, unprivileged
#define MODE_System 0b11111	// thread-mode, privileged
#define MODE_Supervisor 0b10011	// handler-mode (always privileged)

.global __init
__init:

	// https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/cps
	// cps (change processor state) id (interrupt / abort disable)
	cpsid aif, #MODE_System	// disable : imprecise aborts, irq interrupt, fiq interrupt. Enter mode #MODE_System

	ldr r0, =isr_vector

	// arm cortex v8 reference 3-5, table 3-2
	mcr	p15, 0, r0, c12, c0, 0	// set vector base (bit 0 is ignored), move control <- register (mcr)

	// cortex v8, page 3-45 - cache and branch predictor
	mrc	p15, 0, r0, c1, c0, 0	// read system control register
	// data cache, strict alignment, MMU
	orr	r0, r0, #0b101	
	// Instruction cache, Program flow predict 
	orr r0, r0, #(0b11 << 11)
	// enable arm exceptions, clear the 30th bit in r0
	bic r0, r0, #(1 << 30)
	mcr	p15, 0, r0, c1, c0, 0	// system control register

	// L2 cache enable
	mov r1, #1
	mcr p15, 0, r1, c1, c0, 1 // auxiliary control reg

	cpsie	aif			// reenable all interrupts

	ldr	sp, =__StackStart
	
	bl	__BootloaderEntry

idle:
	wfi 		// wait for interrupt
	b	idle
	
# .global setup_vbar
# setup_vbar:
#     ldr r0, =vector_table    @ Load vector table address
#     mcr p15, 0, r0, c12, c0, 0  @ Write to VBAR register
#     bx lr
