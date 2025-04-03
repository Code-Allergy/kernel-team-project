#include <scheduler.h>
#include <types.h> 
#include <ddr.h>
#include <uart.h>
#include <utils.h>
#include <timer.h>
#include <mmu.h>

#define REG_OFFSET     12
#define SP_OFFSET      (REG_OFFSET + 13 * 4)   // 64
#define LR_OFFSET      (SP_OFFSET + 4)         // 68
#define CPSR_OFFSET    (LR_OFFSET + 4)         // 72
#define PC_OFFSET      (CPSR_OFFSET + 4)       // 76


#define MAX_PROCS 64

static scheduler_t scheduler;
static int scheduler_initialized = 0;
volatile int scheduler_tick_flag = 0;
void process1(void);

void recompile(void){
}

int scheduler_should_switch;
process_t *current_process = NULL;

static process_t* proc_table[MAX_PROCESSES];
static int next_proc_index = 0;

volatile uint32_t p2_heartbeat = 0;
__attribute__((naked)) void idle_task() {
   while(1){
   __asm__ volatile (
        "mov r4, pc\n"
        "ldr r5, =0xFFF00000\n"
        "and r4, r4, r5\n"

        "str sp, [r4, #64]\n"
        "str lr, [r4, #68]\n"
        "mrs r6, cpsr\n"
        "str r6, [r4, #72]\n"

        "mov r7, pc\n"
        "str r7, [r4, #76]\n"

        "mov r0, #103\n"
        "swi #0\n"
        :
        :
        : "r0", "r4", "r5", "r6", "r7"
    ); 
   }
}
__attribute__((used)) 
volatile uint32_t idle_task_signature = 0x600D1DEA;


void scheduler_tick(){
    uart_puts("scheduler_tick\n");
   
    uart_printf("P2 Heartbeat: %u\n", p2_heartbeat);

    scheduler_should_switch = 1;
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
    //timer_init(TIMER2, 1000,  scheduler_tick);
    //timer_start(TIMER2);

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

    uart_puts("scheduler start\n");
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

    uart_printf("%x\n", *(uint32_t *)((uint8_t *)p + 64));


    restore_context(p);  // clean handoff, no inline context switch here

    uart_printf("Loaded SP: 0x%x\n", debug_loaded_sp);
    uart_printf("Loaded PC: 0x%x\n", debug_loaded_pc);
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

    uart_puts("Creating Proc\n");

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


    uart_puts("Clearing Segment Memory\n");

    // Clear the segment memory
    ptr = (uint32_t *)paddr;
    for (i = 0; i < SEGMENT_SIZE / sizeof(uint32_t); i++) {
        ptr[i] = 0;
    }



    uart_puts("Post Clear\n");
    

    // Store the process_t struct at the base of the segment
    proc = (process_t *)paddr;
    proc_table[next_proc_index++] = proc;


    uart_puts("Stored process t\n");

    // Code starts near the end of the page (but before the stack)
    code_offset = 0x1000;  // Avoids overwriting struct, still within segment
    code_addr = paddr + code_offset;

    uart_puts("Before getting function size\n");

    func_size = get_function_size(entry_point);
    func_size = (func_size + 7) & ~0x7;  // Round up to nearest 4 bytes


    uart_printf("Received entry_point = 0x%x\n", (uint32_t)entry_point);
    uart_printf("Allocated frame for process at paddr = 0x%x\n", paddr);
    uart_printf("Copying from entry_point = 0x%x to code_addr = 0x%x\n",
                (uint32_t)entry_point, code_addr);

    for (i = 0; i < func_size; i++) {
        ((uint8_t *)code_addr)[i] = ((uint8_t *)entry_point)[i];
    }

    for (i = 0; i < 8; i++) {
        uart_printf("code[0x%x] = 0x%x\n", code_addr + (i * 4), ((uint32_t *)code_addr)[i]);
    }


    for (i = 0; i < 13; i++) {
        proc->registers[i] = 0;
    }

    proc->segment_base    = paddr;
    proc->program_counter = code_addr;
    proc->stack_pointer   = paddr + SEGMENT_SIZE - 0x100;
    proc->state           = READY;
    proc->link_register   = 0x0;
    proc->cpsr            = 0x1F;  // user mode, IRQs disabled

    uart_puts("Process created (with copied code).\n");

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

   
    /* Add processes to the scheduler queue */
    uart_puts("Adding processes to queue\n");
    scheduler_add_process(proc1);

    /* Run the scheduler */
    uart_puts("Running scheduler\n");
    /*scheduler_run();*/
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
