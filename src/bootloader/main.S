#include <memory_map.h>

.cpu	cortex-a8
.text

.global	main
main:
	push	{ lr }

	ldr	r0, =0x44e00000

	movs	r1, #2
	str	r1, [ r0, #CLK_L4LS ]
	orrs	r1, #(1 << 18)		// debounce clock enable
	str	r1, [ r0, #MOD_GPIO1 ]
	str	r1, [ r0, #MOD_GPIO2 ]

1:	ldr	r1, [ r0, #MOD_GPIO1 ]
	tst	r1, #(3 << 16)
	bne	1b

	ldr	r0, =io1_base
	bl	io_reset
	mvns	r1, #(0xf << 21)
	str	r1, [ r0, #IO_nOE ]
	movs	r1, #(1 << 21)
	str	r1, [ r0, #IO_SET ]
loop:
    // Turn LEDs on
    mov	r1, #(0xF << 21)	// LEDs are connected to GPIO1_21 to GPIO1_24
    str	r1, [r0, #0x194]	// Set GPIO1_21 to GPIO1_24 (SETDATAOUT register)

    // Count to a big number
    mov	r2, #0x1FFFFFF
count_on:
    subs	r2, r2, #1
    bne	count_on

    // Turn LEDs off
    str	r1, [r0, #0x190]	// clear GPIO1_21 to GPIO1_24 (CLEARDATAOUT register)

    // Count to a big number
    mov	r2, #0x1FFFFFF
count_off:
    subs	r2, r2, #1
    bne	count_off

    b	loop			// Repeat forever

	pop	{ pc }

// r0	const	gpio controller
// r1	tmp
io_reset:
	movs	r1, #2
	str	r1, [ r0, #IO_SYSCONFIG ]
1:	ldr	r1, [ r0, #IO_SYSSTATUS ]
	tst	r1, #1
	beq	1b

	movs	r1, #0x10
	str	r1, [ r0, #IO_SYSCONFIG ]

	movs	r1, #0
	str	r1, [ r0, #IO_CONTROL ]

	bx lr

