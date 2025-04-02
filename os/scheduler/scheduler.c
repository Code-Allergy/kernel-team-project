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
    /*stack_test();*/
    scheduler_run();
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


void scheduler_run() {
    process_t *p;
    void (*entry)();

    uart_puts("Running minimal scheduler\n");

    p = scheduler.schedule_next();
    if (!p) {
        uart_puts("No process to run.\n");
        while (1);
    }

    uart_printf("Jumping to process PC = 0x%x\n", p->program_counter);

    entry = (void (*)())p->program_counter;
    entry();

    uart_puts("Returned from process (unexpected)\n");
    while (1);
}


process_t* process_create(void (*entry_point)(void)) {
    process_t *proc;
    uint32_t paddr;
    uint32_t *ptr;
    int i;

    paddr = alloc_frame();
    if (!paddr) {
        uart_puts("Error: Failed to allocate memory for process.\n");
        return NULL;
    }

    uart_printf("Allocated frame for process at paddr = 0x%x\n", paddr);

    if (paddr & 0xFFF) {
        uart_puts("Error: paddr is not 4 KB aligned!\n");
        return NULL;
    }

    ptr = (uint32_t *)paddr;
    for (i = 0; i < SEGMENT_SIZE / sizeof(uint32_t); i++) {
        ptr[i] = 0;
    }

    proc = (process_t *)paddr;
    proc->segment_base = paddr;


   uart_printf("process2 is at: 0x%x\n", (uint32_t)&process2);
    uart_printf("Copying from entry_point = 0x%x to paddr = 0x%x\n", (uint32_t)entry_point, paddr);

    // Copy the code into the beginning of the segment
    uart_puts("Copying code into segment\n");
    for (i = 0; i < PROCESS_CODE_SIZE / sizeof(uint32_t); i++) {
        ((uint32_t *)paddr)[i] = ((uint32_t *)entry_point)[i];
    }


    flush_d_cache();
    flush_i_cache();

    // Set PC to the beginning of the copied code
    proc->program_counter = paddr;

    proc->state = READY;

    uart_puts("Process created (with copied code).\n");
    uart_printf("Proc struct at: 0x%x\n", (uint32_t)proc);
    uart_printf("Proc PC set to: 0x%x (copied into segment)\n", proc->program_counter);

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
    while (1) {
        asm volatile("nop");
    }
}

__attribute__((naked)) void process2() {
    while (1) {
        asm volatile("nop");
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
