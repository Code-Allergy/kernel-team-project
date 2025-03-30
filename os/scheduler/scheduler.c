#include <scheduler.h>
#include <types.h> 
#include <ddr.h>
#include <uart.h>
#include <utils.h>
#include <timer.h>
#include <mmu.h>


static scheduler_t scheduler;
static int scheduler_initialized = 0;
volatile int scheduler_tick_flag = 0;
void process1(void);
void process2(void);


process_t *current_process = NULL;



void idle_task() {
    while(1){
        uart_puts("Idle task running...\n");
    }
}


void scheduler_tick(){
    /*scheduler_run() */
    uart_puts("schedule \n");
}


void scheduler_init() {
    int i;
    process_t *idle_proc;

    /* Initialize scheduler state */
    scheduler.current_index = 0;
    scheduler.num_processes = 0;

    /* Set the scheduling algorithm to round robin */
    scheduler.schedule_next = round_robin_scheduler;

    /* Clear process slots */
    for (i = 0; i < MAX_PROCESSES; i++) {
        scheduler.processes[i] = NULL;
    }

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

void scheduler_run() {
    if (!scheduler_initialized){
	uart_puts("uninitialized");
    }

    process_t *old_proc = current_process;
    process_t *new_proc = NULL;


    uart_puts("Called Sched Run \n");


    do {
        new_proc = scheduler.schedule_next();
    } while (new_proc->state == BLOCKED);



    uart_puts("Passed Blocked \n");

    new_proc->state = RUNNING;


    if (old_proc && old_proc->state == RUNNING)
        old_proc->state = READY;

    current_process = new_proc;


 
    uart_puts("Return Proc \n");
}



process_t* round_robin_scheduler() {
    int i;
    process_t *new_proc = NULL;
	
    uart_puts("robin scheduling\n");
    
    for (i = 0; i < scheduler.num_processes; i++) {
        scheduler.current_index = (scheduler.current_index + 1) % scheduler.num_processes;
        if (scheduler.processes[scheduler.current_index]->state == READY) {
            return scheduler.processes[scheduler.current_index];
        }
    }
    return &scheduler.idle_process;
}


process_t* process_create(void (*entry_point)(void)) {
    process_t *proc;
    int i;

    /* Allocate a single frame (1MB segment) for the process */
    uint32_t paddr = alloc_frame();
    if (!paddr) {
        uart_puts("Error: Failed to allocate memory for process.\n");
        return NULL;
    }

    /* Cast the allocated frame as a process_t pointer */
    proc = (process_t *)paddr;

    /* Zero out the entire segment (1MB region) */
    uint32_t *ptr = (uint32_t *)paddr;
    for (i = 0; i < SEGMENT_SIZE / sizeof(uint32_t); i++) {
        ptr[i] = 0;
    }

    /* Set segment base to the allocated frame address */
    proc->segment_base = paddr;

    /* Set up user stack */
    proc->stack_pointer = paddr + USER_STACK_TOP;  
    uint32_t *stack = (uint32_t *)proc->stack_pointer;

    /* Initialize stack with initial context */
    *(--stack) = (uint32_t)entry_point;    /* PC (entry point) */
    *(--stack) = (uint32_t)scheduler_run;  /* LR fallback */

    /* Clear R0-R12 */
    for (i = 12; i >= 0; i--) {
        *(--stack) = 0;
    }

    /* Set stack pointer to the updated stack */
    proc->stack_pointer = (uint32_t)stack;

    /* Set kernel stack pointer (optional, if needed) */
    proc->link_register = (uint32_t)entry_point;
    proc->cpsr = 0x600001D3;  /* System mode, interrupts enabled */
    proc->current_mode = 1;

    uart_puts("Process created.\n");
    return proc;
}



void boot_test() {
    process_t *proc1, *proc2;

    uart_puts("boot_test_start\n");

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
    scheduler_run(); 
}


void yield() {
    scheduler.processes[scheduler.current_index]->state = READY;
    scheduler_run(); /* explicitly yield back to scheduler */
}



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
