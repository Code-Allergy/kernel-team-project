#include <scheduler.h>
#include <types.h> 
#include <ddr.h>
#include <uart.h>
#include <utils.h>
#include <timer.h>
#include <mmu.h>
#include <circular_buffer.h>

#define REG_OFFSET     12
#define SP_OFFSET      (REG_OFFSET + 13 * 4)   // 64
#define LR_OFFSET      (SP_OFFSET + 4)         // 68
#define CPSR_OFFSET    (LR_OFFSET + 4)         // 72
#define PC_OFFSET      (CPSR_OFFSET + 4)       // 76


#define MAX_PROCS 64

static scheduler_t scheduler;
static generic_circular_buffer_t processes;
static generic_circular_buffer_t free_processes;
static generic_circular_buffer_t ready_queue;
static int scheduler_initialized = 0;
volatile int scheduler_tick_flag = 0;
void process1(void);

void recompile(void){
}

int scheduler_should_switch;
process_t *current_process = NULL;

static process_t* proc_table[MAX_PROCESSES];
static int next_proc_index = 0;

volatile uint32_t p2_heartbeat = 0, idle_proc_heartbeat = 0, idle_count = 0;
__attribute__((naked)) void idle_task() {
    uart_printf("Idle task running...\n");
    while(1){
        uart_printf("Idle task heartbeat: %d\n", idle_proc_heartbeat);
        idle_count = 0x1FFFFF;
        while (idle_count > 0) {
            idle_count--;
        }
        idle_proc_heartbeat++;
    }
}
__attribute__((used)) 
volatile uint32_t idle_task_signature = 0x600D1DEA;


void scheduler_tick(){
    uart_puts("scheduler_tick\n");
   
    uart_printf("P2 Heartbeat: %u\n", p2_heartbeat);

    scheduler_should_switch = 1;
}

void snprintf(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

volatile uint32_t debug_loaded_pc = 0;
volatile uint32_t debug_loaded_sp = 0;

__attribute__((naked)) void restore_context(process_t *proc) {
    __asm__ volatile (
        "mov r1, r0\n"                             // r1 = proc

        // Load SP
        "ldr r2, [r1, %[sp_offset]]\n"
        "mov sp, r2\n"

        // Set CPSR to User mode (from proc->cpsr)
        "ldr r2, [r1, %[cpsr_offset]]\n"
        "msr spsr_cxsf, r2\n"

        // Load PC
        "ldr r2, [r1, %[pc_offset]]\n"

        // Jump to user mode
        "movs pc, r2\n"  // triggers mode switch from SPSR

        :
        : [pc_offset] "I" (PC_OFFSET),
          [sp_offset] "I" (SP_OFFSET),
          [cpsr_offset] "I" (CPSR_OFFSET)
        : "r1", "r2", "memory"
    );
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
    process_t *idle_proc, *proc;

    check_mode();

    /* Initialize scheduler state */
    scheduler.current_index = 0;
    scheduler.num_processes = 0;
    scheduler_should_switch = 1;

    /* Initialize the circular buffer for processes */
    generic_circular_buffer_init(&processes);
    generic_circular_buffer_init(&free_processes);
    generic_circular_buffer_init(&ready_queue);

    /* Populate free processes ring buffer */
    for (i = 0; i < MAX_PROCESSES; i++) {
        proc = &scheduler.processes[i];
        proc->pid = i;
        proc->state = UNDEFINED;
        proc->priority = 0;
        proc->program_counter = 0;
        proc->stack_pointer = 0;
        proc->link_register = 0;
        proc->cpsr = 0;
        generic_circular_buffer_push(&free_processes, (void*)(proc));
    }

    /* Create idle process */
    uart_puts("Creating idle process...\n");
    idle_proc = process_create(idle_task);
    if (!idle_proc) {
        uart_puts("Failed to create idle process\n");
        while (1);
    }
    snprintf(idle_proc->name, "Idle Process", sizeof(idle_proc->name));
    uart_printf("Created process: %s\n", idle_proc->name);

    generic_circular_buffer_push(&processes, (void*)idle_proc);
    scheduler.idle_process = idle_proc;
    //generic_circular_buffer_push(&ready_queue, (void*)idle_proc);

    /* Use Timer 2 for scheduling */
    // timer_init(TIMER2, 1000, scheduler_tick);
    // timer_start(TIMER2);
    // uart_puts("Timer started\n");

    uart_puts("Scheduler Initialized\n");
}



int scheduler_add_process(process_t *proc) {
    // if (!scheduler_initialized) {
    //     uart_puts("Error: Cannot add process, scheduler is not initialized!\n");
    //     return -1;
    // }

    // if (scheduler.num_processes >= MAX_PROCESSES) {
    //     return -1;  /* No space for new processes */
    // }

    // scheduler.processes[scheduler.num_processes] = proc;
    // proc->state = READY;
    // scheduler.num_processes++;
    return 0;
}






process_t* round_robin_scheduler() {
    // int i;
    // process_t *new_proc = NULL;
	
    // uart_puts("robin scheduling\n");
    
    // for (i = 0; i < scheduler.num_processes; i++) {
    //     scheduler.current_index = (scheduler.current_index + 1) % scheduler.num_processes;
    //     process_t *candidate = scheduler.processes[scheduler.current_index];
    //     if (candidate && candidate->state == READY) {
    //         return candidate;
    //     }
    // }

    return &scheduler.idle_process;
}

void scheduler_run() {
    process_t *next_proc;
    bool status;

    uart_puts("Scheduler running...\n");

    status = generic_circular_buffer_pop(&ready_queue, (void**)&next_proc);
    if (!status) {
        uart_puts("No READY process. Running idle...\n");
        next_proc = scheduler.idle_process;
    }

    current_process = next_proc;
    current_process->state = RUNNING;
    uart_printf("Scheduler starting proc: %s, pc: %x \n", current_process->name, current_process->program_counter);
    restore_context_asm(); // Jump to the process
    uart_printf("Ummm... we should not be here\n");
    while(1);

    __builtin_unreachable();
}

process_t* process_create(void (*entry_point)(void)) {
    int i;
    process_t *proc;
    bool status;

    status = generic_circular_buffer_pop(&free_processes, (void**)&proc);
    if (!status) {
        uart_puts("No free process slots available\n");
        return NULL;
    }
    proc->state = NEW;
    proc->priority = 1; // Default priority, can be modified
    proc->program_counter = (uint32_t)entry_point;
    proc->segment_base = (uint32_t)proc;
    /* processes will be run in system mode since 
        we are currently having issues context switching betweeen
        system and user mode with the MMU*/
    proc->current_mode = MODE_System | I_F_BIT;
    proc->cpsr = MODE_System | I_F_BIT;
    proc->shared_page = 0;

    /*  Clear registers */
    for (i = 0; i < 16; i++) {
        proc->registers[i] = i;
    }

    proc->link_register = 0;

    /* Setup the user stack */
    proc->stack_top = (uint32_t)(proc->stack + USER_STACK_SIZE);
    proc->stack_pointer = proc->stack_top;

    //proc->state = READY;
    scheduler.num_processes++;

    /* proc->pid set already in scheduler_init() */

    uart_puts("Creating Proc\n");

    return proc;
}

// void print_cpu_regs(void){
//     __asm__ volatile (
//         "mov r0, r1\n"                             // r1 = proc

//         // Load SP
//         "ldr r2, [r0, %[sp_offset]]\n"
//         "mov sp, r2\n"

//         // Set CPSR to User mode (from proc->cpsr)
//         "ldr r2, [r0, %[cpsr_offset]]\n"
//         "msr spsr_cxsf, r2\n"

//         // Load PC
//         "ldr r2, [r0, %[pc_offset]]\n"

//         // Jump to user mode
//         "movs pc, r2\n"  // triggers mode switch from SPSR

//         :
//         : [pc_offset] "I" (PC_OFFSET),
//           [sp_offset] "I" (SP_OFFSET),
//           [cpsr_offset] "I" (CPSR_OFFSET)
//         : "r1", "r2", "memory"
//     );
// }


void boot_test() {
}


void process1() {
    volatile uint32_t p1_counter = 0;

    while (1) {
        p1_counter++;
        if (p1_counter > 3000000) {
            p1_counter = 0;

        }
    }
}


__attribute__((used)) 
volatile uint32_t process1_signiture = 0x600D1DEA;

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
