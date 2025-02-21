.arm

// these are not real
#define MODE_USR 0b10000	// thread-mode, unprivileged
#define MODE_THR 0b11111	// thread-mode, privileged
#define MODE_HND 0b11111	// handler-mode (always privileged)

.global __init
__init:
	// minimalistic cortex-a8 initialization
	subs	r4, lr, #4	// vector base, except bit 0 indicates thumb

	cpsid	fia, #MODE_THR	// thread mode, mask all async exceptions

	ldr	sp, =__StackStart

	mcr	p15, 0, r4, c12, c0, 0	// set vector base (bit 0 is ignored)

	mov	r0, #(3 << 11)		// enable branch prediction and i-cache
	bfi	r0, r4, #30, #1		// configure arm/thumb-mode exceptions
	mcr	p15, 0, r0, c1, c0, 0	// system control register

	cpsie	a			// enable async aborts

	bl	__BootloaderEntry
idle:
	wfi
	b	idle
	
# .global setup_vbar
# setup_vbar:
#     ldr r0, =vector_table    @ Load vector table address
#     mcr p15, 0, r0, c12, c0, 0  @ Write to VBAR register
#     bx lr
