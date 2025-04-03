#include <scheduler.h>
#include <types.h> 
#include <ddr.h>
#include <uart.h>
#include <utils.h>
#include <timer.h>
#include <mmu.h>

#define REG_OFFSET     8
#define SP_OFFSET      (REG_OFFSET + 13 * 4)
#define LR_OFFSET      (REG_OFFSET + 13 * 4)
#define CPSR_OFFSET    (REG_OFFSET + 14 * 4)
#define PC_OFFSET      (REG_OFFSET + 15 * 4)


#define MAX_PROCS 64

static scheduler_t scheduler;
static int scheduler_initialized = 0;
volatile int scheduler_tick_flag = 0;
void process1(void);
void process2(void);

void recompile(void){
}

int scheduler_should_switch;
process_t *current_process = NULL;

static process_t* proc_table[MAX_PROCESSES];
static int next_proc_index = 0;

__attribute__((naked)) void idle_task() {
    asm volatile(
        "1:\n"
        "nop\n"
        "b 1b\n"
    );
}


void scheduler_tick(){
    /*stack_test();*/
    scheduler_should_switch = 1;
}

__attribute__((naked)) void do_restore_context(uint32_t *regs, uint32_t sp,
                                               uint32_t lr_val, uint32_t cpsr,
                                               uint32_t pc_val) {
    __asm__ volatile (
        "ldmia r0, {r0-r12}\n"        // Load saved registers
        "mov sp, r1\n"                // Set stack pointer
        "mov lr, r2\n"                // Set LR
        "msr spsr_cxsf, r3\n"         // Restore CPSR
        "movs pc, r4\n"               // Return to user mode
    );
}

void restore_context(process_t *proc) {
    uint32_t *regs = proc->registers;
    uint32_t sp = proc->stack_pointer;
    uint32_t lr = proc->link_register;
    uint32_t cpsr = proc->cpsr;
    uint32_t pc = proc->program_counter;



    do_restore_context(regs, sp, lr, cpsr, pc);
}



unsigned int get_function_size(void *func) {
    uint32_t *ptr = (uint32_t *)func;
    unsigned int count = 0;
    while (ptr[count] != 0x600D1DEA) {
        count++;
    }
    return count * sizeof(uint32_t);  // returns size up to (but NOT including) sentinel
}



void scheduler_init() {
    int i;
    process_t *idle_proc;

    check_mode();

    /* Initialize scheduler state */
    scheduler.current_index = 0;
    scheduler.num_processes = 0;
    scheduler_should_switch = 1;

    /* Set the scheduling algorithm to round robin */
    scheduler.schedule_next = round_robin_scheduler;

    /* Clear process slots */
    for (i = 0; i < MAX_PROCESSES; i++) {
        scheduler.processes[i] = NULL;
    }


    uart_printf("Passing idle_task address: 0x%x\n", (uint32_t)&idle_task);

    check_mode();
    /* Create Idle Process */
    idle_proc = process_create(idle_task);
    if (!idle_proc) {
        uart_puts("Error: Failed to create idle process.\n");
        return;
    }



    /* Set idle process as the fallback */
    scheduler.idle_process = *idle_proc;
    scheduler.idle_process.pid = 0;
    scheduler.idle_process.priority = 0;

    /* Free the dynamically allocated idle_proc frame after copying */
    free_frame((uint32_t)idle_proc);

    uart_puts("making timer 3 \n");

    current_process = &scheduler.idle_process;


    /* Set up timer to trigger scheduler every 1000ms */
    timer_init(TIMER2, 1000,  scheduler_tick);
    timer_start(TIMER2);

    /* Mark scheduler as initialized */
    scheduler_initialized = 1;

    uart_puts("Scheduler Initialized\n");
}



int scheduler_add_process(process_t *proc) {
    if (!scheduler_initialized) {
        uart_puts("Error: Cannot add process, scheduler is not initialized!\n");
        return -1;
    }

    if (scheduler.num_processes >= MAX_PROCESSES) {
        return -1;  /* No space for new processes */
    }

    scheduler.processes[scheduler.num_processes] = proc;
    proc->state = READY;
    scheduler.num_processes++;
    return 0;
}






process_t* round_robin_scheduler() {
    int i;
    process_t *new_proc = NULL;
	
    uart_puts("robin scheduling\n");
    
    for (i = 0; i < scheduler.num_processes; i++) {
        scheduler.current_index = (scheduler.current_index + 1) % scheduler.num_processes;
        process_t *candidate = scheduler.processes[scheduler.current_index];
        if (candidate && candidate->state == READY) {
            return candidate;
        }
    }

    return &scheduler.idle_process;
}

void scheduler_run() {
    if (!scheduler_should_switch) {
        // No scheduling needed, just resume current process
        restore_context(current_process);
        __builtin_unreachable();  // for clarity
    }

    scheduler_should_switch = 0;  // reset for next tick

    process_t *old_proc = current_process;
    process_t *p = scheduler.schedule_next();

    if (!p) {
        uart_puts("No process to run.\n");
        while (1);
    }

    if (old_proc && old_proc->state == RUNNING) {
        old_proc->state = READY;
    }

    p->state = RUNNING;
    current_process = p;

    uart_puts("restoring\n");

    uart_printf("restoring: proc = 0x%x\n", (uint32_t)p);
    uart_printf("          proc->lr addr = 0x%x\n", (uint32_t)&(p->link_register));



    uart_printf("restoring: proc = 0x%x\n", (uint32_t)p);
    uart_printf("  pc = 0x%x\n", p->program_counter);
    uart_printf("  sp = 0x%x\n", p->stack_pointer);
    uart_printf("  lr = 0x%x\n", p->link_register);
    uart_printf("  cpsr = 0x%x\n", p->cpsr);

    uart_printf("  raw PC = 0x%x\n", *(uint32_t *)((uint8_t *)p + PC_OFFSET));

    restore_context(p);  // clean handoff, no inline context switch here
    __builtin_unreachable();
}





process_t* process_create(void (*entry_point)(void)) {
    int i;
    uint32_t paddr;
    uint32_t code_offset;
    uint32_t code_addr;
    unsigned int func_size;
    uint32_t *ptr;
    process_t *proc;

    if (next_proc_index >= MAX_PROCESSES) {
        uart_puts("No more process slots available.\n");
        return NULL;
    }

    paddr = alloc_frame();
    if (!paddr) {
        uart_puts("Error: Failed to allocate memory for process.\n");
        return NULL;
    }

    if (paddr & 0xFFF) {
        uart_puts("Error: paddr is not 4 KB aligned!\n");
        return NULL;
    }

    // Clear the segment memory
    ptr = (uint32_t *)paddr;
    for (i = 0; i < SEGMENT_SIZE / sizeof(uint32_t); i++) {
        ptr[i] = 0;
    }

    // Store the process_t struct at the base of the segment
    proc = (process_t *)paddr;
    proc_table[next_proc_index++] = proc;

    // Code starts after the struct (at 0x1000 offset)
    code_offset = 0x1000;
    code_addr = paddr + code_offset;
    func_size = get_function_size(entry_point);

    uart_printf("Received entry_point = 0x%x\n", (uint32_t)entry_point);
    uart_printf("Allocated frame for process at paddr = 0x%x\n", paddr);
    uart_printf("Copying from entry_point = 0x%x to code_addr = 0x%x\n",
                (uint32_t)entry_point, code_addr);

    for (i = 0; i < func_size / sizeof(uint32_t); i++) {
        ((uint32_t *)code_addr)[i] = ((uint32_t *)entry_point)[i];
    }

    uart_printf("Verifying copy at code_addr: 0x%x\n", code_addr);
    for (i = 0; i < 4; i++) {
        uart_printf("word[%d] = 0x%x\n", i, ((uint32_t *)code_addr)[i]);
    }

    for (i = 0; i < 13; i++) {
        proc->registers[i] = 0;
    }

    proc->segment_base    = paddr;
    proc->program_counter = code_addr;
    proc->stack_pointer   = paddr + SEGMENT_SIZE - 0x100;
    proc->state           = READY;
    proc->link_register   = 0x0;
    proc->cpsr            = 0x10;  // user mode, IRQs disabled

    uart_puts("Process created (with copied code).\n");
    uart_printf("Proc PC set to: 0x%x\n", proc->program_counter);

    return proc;
}










void boot_test() {
    process_t *proc1, *proc2;

    uart_puts("boot_test_start\n");


    uart_printf("Passing process1 address: 0x%x\n", (uint32_t)&process1);

    /* Create process 1 */
    uart_puts("Creating proc 1\n");
    proc1 = process_create(process1);
    if (!proc1) {
        uart_puts("Error: Failed to create proc1\n");
        return;
    }


    /* Create process 2 */
    uart_puts("Created 1, creating proc 2\n");
    proc2 = process_create(process2);
    if (!proc2) {
        uart_puts("Error: Failed to create proc2\n");
        /* Free proc1 if proc2 creation fails */
        free_frame((uint32_t)proc1);
        return;
    }

   
    /* Add processes to the scheduler queue */
    uart_puts("Adding processes to queue\n");
    scheduler_add_process(proc1);
    scheduler_add_process(proc2);

    /* Run the scheduler */
    uart_puts("Running scheduler\n");
    /*scheduler_run();*/
}


void yield() {
    scheduler.processes[scheduler.current_index]->state = READY;
    scheduler_run(); /* explicitly yield back to scheduler */
}


/*
void process1() {                                                               
    int i;
    while (1) {
        uart_puts("Process 1 running...\n");                                        
    	for(i = 0; i < 10000; i++);
    }                                                                
}
                                                                               
void process2() {                                                               
    int i;
    while (1) {
        uart_puts("Process 2 running...\n"); 
    	for(i = 0; i < 10000; i++);
    }                                                                
}
*/


__attribute__((naked)) void process1() {
    asm volatile (
        "b 1f\n"
        ".word 0x600D1DEA\n"
        "1:\n"
        "b 1b\n"
    );
}



__attribute__((used))
volatile uint32_t process1_signature = 0x600D1DEA;


__attribute__((naked)) void process2() {
    asm volatile (
        "1:\n"
        "b 1b\n"                 // Infinite loop
        ".align 4\n"
        ".global __end_process2\n"
        "__end_process2:\n"
        ".word 0x600D1DEA\n"     // This marker is *data*, not code
    );
}





__attribute__((used))
volatile uint32_t process2_signature = 0x600D1DEA;



                                                                               


void uart_print_hex(uint32_t value) {
    char hex_string[9]; /* 8 characters + null terminator */
    int i;
    
    hex_string[8] = '\0'; /* Null-terminate the string */

    for (i = 7; i >= 0; i--) {
        int digit = value & 0xF; /* Extract the last 4 bits */
        hex_string[i] = (digit < 10) ? ('0' + digit) : ('A' + (digit - 10));
        value >>= 4; /* Shift right by 4 bits */
    }

    uart_puts("0x");
    uart_puts(hex_string);
}
