.section .text
.global get_irq_sp
.global restore_context_asm
.global save_context_asm
.global os_sp_cache
.global usr_sp_cache
.global save_sp_and_lr_usr
.global load_os_sp
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
usr_sp_cache:
    .space 4                       @ Allocate 4 bytes for user stack cache
irq_lr_cache:
    .space 4                       @ Allocate 4 bytes for IRQ LR cache
                                   @ as calling save_context_asm modifies LR
@ r0 =  sp
@ r1 =  lr
save_sp_and_lr_usr:
    str    r0, usr_sp_cache         @ Save user stack pointer to usr_sp_cache
    str    r1, irq_lr_cache         @ Save IRQ LR to irq_lr_cache
    bx    lr                        @ Return to caller (irq or swi top half)

load_os_sp:
    ldr   r13, os_sp_cache     @ Load OS stack pointer from os_sp_cache
    bx    lr                   @ Return to caller (irq or swi top half)

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

save_context_asm:
    @ save r0 and r1 so that we can use them
    stmfd  sp!, {r0}                @ Push r0 to the stack
    stmfd  sp!, {r1}                @ Push r1 to the stack
    ldr    r0, =current_process     @ r0 = &current_process
    ldr    r0, [r0]                 @ r0 = current_process

    @ Save r2-r12 to pcb
    mov     r1, r2
    str     r1, [r0, #R2_OFF]
    mov     r1, r3
    str     r1, [r0, #R3_OFF]
    mov     r1, r4
    str     r1, [r0, #R4_OFF]
    mov     r1, r5
    str     r1, [r0, #R5_OFF]
    mov     r1, r6
    str     r1, [r0, #R6_OFF]
    mov     r1, r7
    str     r1, [r0, #R7_OFF]
    mov     r1, r8
    str     r1, [r0, #R8_OFF]
    mov     r1, r9
    str     r1, [r0, #R9_OFF]
    mov     r1, r10
    str     r1, [r0, #R10_OFF]
    mov     r1, r11
    str     r1, [r0, #R11_OFF]
    mov     r1, r12
    str     r1, [r0, #R12_OFF]

    @ Save user-mode SP
    ldr     r1, usr_sp_cache
    str     r1, [r0, #SP_OFF]

    @ Save user-mode LR
    ldr     r1, irq_lr_cache
    str     r1, [r0, #LR_OFF]

    @ because we are here because and irq or swi occured, lr is pc
    @ Save PC (same as LR)
    str     r1, [r0, #PC_OFF]

    @ Save CPSR
    mrs     r1, spsr                  @ r1 = spsr 
    str     r1, [r0, #CPSR_OFF]

    @ last r0 and r1 are saved in the stack
    ldmfd   sp!, {r1}                @ Pop r1 from the stack
    str     r1, [r0, #R0_OFF]        @ Save r1 to pcb
    ldmfd   sp!, {r1}                @ Pop r0 from the stack
    str     r1, [r0, #R1_OFF]        @ Save r0 to pcb

    bx      lr                       @ Return to caller (irq or swi top half)
