.section .text
.global get_irq_sp
.global restore_context_asm
.extern current_process

#define R0_OFF        0    /* Offset for register r0*/
#define R1_OFF        4    /* Offset for register r1*/
#define R2_OFF        8    /* Offset for register r2*/
#define R3_OFF        12   /* Offset for register r3*/
#define R4_OFF        16   /* Offset for register r4*/
#define R5_OFF        20   /* Offset for register r5*/
#define R6_OFF        24   /* Offset for register r6*/
#define R7_OFF        28   /* Offset for register r7*/
#define R8_OFF        32   /* Offset for register r8*/
#define R9_OFF        36   /* Offset for register r9*/
#define R10_OFF       40   /* Offset for register r10*/
#define R11_OFF       44   /* Offset for register r11*/
#define R12_OFF       48   /* Offset for register r12*/

#define SP_OFF        52   /* Offset for stack pointer (r13)*/
#define LR_OFF        56   /* Offset for link register (r14)*/
#define CPSR_OFF      60   /* Offset for CPSR*/
#define PC_OFF        64   /* Offset for program counter (r15)*/

sched_halt:
    nop                             @ Infinite loop
    b sched_halt                   @ Infinite loop

@ This is a bit hacky
@ Allocate a word to cache the pc during ctx switch
pc_cache:
    .space 4                       @ Allocate 4 bytes for pc cache
os_sp_cache:
    .space 4                       @ Allocate 4 bytes for os stack cache

restore_context_asm:
    ldr     r0, =current_process     @ r0 = &current_process
    ldr     r0, [r0]                 @ r0 = current_process

    @ Load PC from process_t save it
    ldr     r1, [r0, #PC_OFF]
    str     r1, pc_cache             @ Save PC to pc_cache
    //stmfd   sp!, {r1}                @ Push PC to the stack
    @ SP
    ldr     r1, [r0, #SP_OFF]
    stmfd   sp!, {r1}                @ Push SP to the stack
    @ LR
    ldr     r1, [r0, #LR_OFF]
    stmfd   sp!, {r1}                @ Push LR to the stack
    @ r12
    ldr     r1, [r0, #R12_OFF]
    stmfd   sp!, {r1}                @ Push r12 to the stack
    @ r11
    ldr     r1, [r0, #R11_OFF]
    stmfd   sp!, {r1}                @ Push r11 to the stack
    @ r10
    ldr     r1, [r0, #R10_OFF]
    stmfd   sp!, {r1}                @ Push r10 to the stack
    @ r9
    ldr     r1, [r0, #R9_OFF]
    stmfd   sp!, {r1}                @ Push r9 to the stack
    @ r8
    ldr     r1, [r0, #R8_OFF]
    stmfd   sp!, {r1}                @ Push r8 to the stack
    @ r7
    ldr     r1, [r0, #R7_OFF]
    stmfd   sp!, {r1}                @ Push r7 to the stack
    @ r6
    ldr     r1, [r0, #R6_OFF]
    stmfd   sp!, {r1}                @ Push r6 to the stack
    @ r5
    ldr     r1, [r0, #R5_OFF]
    stmfd   sp!, {r1}                @ Push r5 to the stack
    @ r4
    ldr     r1, [r0, #R4_OFF]
    stmfd   sp!, {r1}                @ Push r4 to the stack
    @ r3
    ldr     r1, [r0, #R3_OFF]
    stmfd   sp!, {r1}                @ Push r3 to the stack
    @ r2
    ldr     r1, [r0, #R2_OFF]
    stmfd   sp!, {r1}                @ Push r2 to the stack
    @ r1
    ldr     r1, [r0, #R1_OFF]
    stmfd   sp!, {r1}                @ Push r1 to the stack
    @ r0
    ldr     r1, [r0, #R0_OFF]
    stmfd   sp!, {r1}                @ Push r0 to the stack

    @ Load the process registers from the stack
    ldmfd   sp!, {r0}                @ Pop r0 from the stack
    ldmfd   sp!, {r1}                @ Pop r1 from the stack
    ldmfd   sp!, {r2}                @ Pop r2 from the stack
    ldmfd   sp!, {r3}                @ Pop r3 from the stack
    ldmfd   sp!, {r4}                @ Pop r4 from the stack
    ldmfd   sp!, {r5}                @ Pop r5 from the stack
    ldmfd   sp!, {r6}                @ Pop r6 from the stack
    ldmfd   sp!, {r7}                @ Pop r7 from the stack
    ldmfd   sp!, {r8}                @ Pop r8 from the stack
    ldmfd   sp!, {r9}                @ Pop r9 from the stack
    ldmfd   sp!, {r10}               @ Pop r10 from the stack
    ldmfd   sp!, {r11}               @ Pop r11 from the stack
    ldmfd   sp!, {r12}               @ Pop r12 from the stack
    ldmfd   sp!, {r14}               @ Pop r14 (lr) from the stack

    @ Save OS stack pointer
    str    r13, os_sp_cache             @ Save OS stack pointer to os_sp_cache

    ldmfd   sp!, {r13}               @ Pop r13 (sp) from the stack
    @ get the pc from the cache
    ldr     r15, pc_cache            @ Load PC from pc_cache
    
    b sched_halt

    //ldr     r2, =0x2
    //stmfd   sp!, {r2} 
    //ldmfd   sp!, {r8}
    //ldmfd   sp!, {r9}
    //mov     r0, r9                   @ r0 = PC
    //bl uart_print_hex
    //b sched_halt
/*
    @ Use r0 as base = &registers[0]
    add     r0, r0, #0               @ r0 = &process->registers[0] (offset 0)

    @ Push r0-r12 to the stack
    ldmia   r0!, {r2-r12}            @ Load r0 to r12 from registers
    stmfd   sp!, {r2-r12}            @ Push r2-r12 to the stack

    @ Restore LR
    ldr     r2, [r0, #56]            @ Link Register is at offset 56
    mov     lr, r2                   @ Restore link register to lr

    @ Restore CPSR
    ldr     r2, [r0, #60]            @ CPSR is at offset 60
    msr     spsr_cxsf, r2            @ Restore CPSR to SPSR

    @ Pop r0-r12 from the stack
    ldmfd   sp!, {r0-r12}            @ Pop r0-r12 from the stack

    @ Pop PC from the stack (this will return to the process)
    ldmfd   sp!, {r8}                @ Pop the PC from the stack into r8
    mov     pc, r8                   @ Restore the PC, and the context switch is complete
*/


@ r1 is offset into pcb
.extern uart_print_hex

print_reg:
    ldr    r0, =current_process     @ r0 = &current_process
    ldr    r0, [r0]                 @ r0 = current_process
    add    r0, r0, r1               @ r0 = process->registers + offset
    ldr    r0, [r0]                 @ r0 = process->registers[reg]
    bl     uart_print_hex           @ Call print_hex to print r0 (r1 in hex)
    bx     lr                       @ Return to caller

.global print_pcb_values
print_pcb_values:
    ldr     r0, =current_process     @ r0 = &current_process
    ldr     r0, [r0]                 @ r0 = current_process

    @ Print r0
    ldr     r1, =#12            @ r1 = process->registers[0] (offset 12)
    bl      print_reg                @ Call print_hex to print r1 (r0 in hex)

    @ Print r1
    ldr     r1, =#16            @ r1 = process->registers[1] (offset 16)
    bl      print_reg                @ Call print_hex to print r1 (r1 in hex)

    @ Print r2
    ldr     r1, =#20            @ r1 = process->registers[2] (offset 20)
    bl      print_reg                @ Call print_hex to print r1 (r2 in hex)

    @ Print r3
    ldr     r1, =#24            @ r1 = process->registers[3] (offset 24)
    bl      print_reg                @ Call print_hex to print r1 (r3 in hex)

    @ Print r4
    ldr     r1, =#28            @ r1 = process->registers[4] (offset 28)
    bl      print_reg                @ Call print_hex to print r1 (r4 in hex)

    @ Print r5
    ldr     r1, =#32            @ r1 = process->registers[5] (offset 32)
    bl      print_reg                @ Call print_hex to print r1 (r5 in hex)

    @ Print r6
    ldr     r1, =#36            @ r1 = process->registers[6] (offset 36)
    bl      print_reg                @ Call print_hex to print r1 (r6 in hex)

    @ Print r7
    ldr     r1, =#40            @ r1 = process->registers[7] (offset 40)
    bl      print_reg                @ Call print_hex to print r1 (r7 in hex)

    @ Print r8
    ldr     r1, =#44            @ r1 = process->registers[8] (offset 44)
    bl      print_reg                @ Call print_hex to print r1 (r8 in hex)

    @ Print r9
    ldr     r1, =#48            @ r1 = process->registers[9] (offset 48)
    bl      print_reg                @ Call print_hex to print r1 (r9 in hex)

    @ Print r10
    ldr     r1, =#52            @ r1 = process->registers[10] (offset 52)
    bl      print_reg                @ Call print_hex to print r1 (r10 in hex)

    @ Print r11
    ldr     r1, =#56            @ r1 = process->registers[11] (offset 56)
    bl      print_reg                @ Call print_hex to print r1 (r11 in hex)

    @ Print r12
    ldr     r1, =#60            @ r1 = process->registers[12] (offset 60)
    bl      print_reg                @ Call print_hex to print r1 (r12 in hex)

    @ Print Link Register (LR)
    ldr     r1, =#64            @ r1 = process->link_register (offset 64)
    bl      print_reg                @ Call print_hex to print r1 (LR in hex)

    @ Print Program Counter (PC)
    ldr     r1, =#68            @ r1 = process->program_counter (offset 68)
    bl      print_reg                @ Call print_hex to print r1 (PC in hex)

    b .                              @ Return from print_pcb_values

get_irq_sp:
    mrs r0, cpsr              @ Save current mode
    orr r1, r0, #0x12         @ Switch to IRQ mode
    msr cpsr_c, r1            @ Set IRQ mode
    mov r0, sp                @ Load IRQ stack pointer into r0
    msr cpsr_c, r0            @ Restore original mode
    bx lr                     @ Return to caller
