
// place in special section to allow linker script to place it at beginning
.section .text._start

// force ARM mode (required by ROM booloader)
.arm

// entrypoint
.global _start
_start:
	bl init

.global setup_vbar
setup_vbar:
    ldr r0, =vector_table    @ Load vector table address
    mcr p15, 0, r0, c12, c0, 0  @ Write to VBAR register
    bx lr
