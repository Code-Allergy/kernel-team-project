.global _start
.section .text

.include "mem_addresses_qemu.h"

_start:
    ldr sp, =_stack_start
    bl uart_test
    b .

