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



void stack_test() {                                                            
    int i;                                                                              
    if (!scheduler_initialized) {                                                
        uart_puts("uninitialized");                                              
        return;                                                                  
    }                                                                            
                                                                                 
    process_t *old_proc = current_process;                                        
    process_t *new_proc = NULL;                                                  
    unsigned int raw_stack_sys[512];  /* System/User mode stack */               
    unsigned int raw_stack_irq[512];  /* IRQ mode stack */                       
    unsigned int *stack_ptr_sys = (unsigned int *)raw_stack_sys;                 
    unsigned int *stack_ptr_irq = (unsigned int *)raw_stack_irq;                 
    unsigned int irq_sp;                                                         
                                                                                 
    uart_puts("Called Sched Run\n");                                             
                                                                                 
    /* Dump System/User stack and search for 0xCAFEBABE */                       
    asm volatile (                                                               
        "mov r1, sp\n"                  /* Load System/User stack pointer */   
        "mov r2, %[stack_ptr_sys]\n"    /* Load raw_stack_sys pointer */       
        "mov r3, #512\n"                /* Number of words to pop */           
        "ldr r12, =0xCAFEBABE\n"        /* Load marker to check */            
                                                                                 
        "1:\n"                                                                  
        "ldr r0, [r1], #4\n"            /* Load word from r1 and advance */   
        "str r0, [r2], #4\n"            /* Store word in raw_stack_sys */     
        "cmp r0, r12\n"                 /* Compare with marker */             
        "beq loop_forever_sys\n"        /* If found, loop forever */          
        "subs r3, r3, #1\n"                                                   
        "bne 1b\n"                      /* Loop until counter hits 0 */       
        "b continue_sys\n"              /* Continue if marker not found */    
                                                                                 
    "loop_forever_sys:\n"                                                      
        "b loop_forever_sys\n"          /* Infinite loop if marker found */   
                                                                                 
    "continue_sys:\n"                                                          
        :                                                                        
        : [stack_ptr_sys] "r" (stack_ptr_sys)                                   
        : "r0", "r1", "r2", "r3", "r12", "memory"                     
    );                                                                           
                                                                                 
    /* Print System/User stack in address:value format */                        
    for (i = 0; i < 512; i++) {                                                  
        uart_printf("Address: 0x%x, Value: 0x%x\n",                           
                    (unsigned int)&raw_stack_sys[i], raw_stack_sys[i]);          
    }                                                                            
                                                                                 
    /* Get IRQ stack pointer */                                                  
    asm volatile (                                                               
        "mrs r0, cpsr\n"                /* Save current mode */               
        "orr r1, r0, #0x12\n"           /* Switch to IRQ mode */               
        "msr cpsr_c, r1\n"              /* Set IRQ mode */                    
        "mov %[irq_sp], sp\n"           /* Get IRQ stack pointer */           
        "msr cpsr_c, r0\n"              /* Restore original mode */           
        : [irq_sp] "=r" (irq_sp)                                                
        :                                                                        
        : "r0", "r1", "memory"                                               
    );                                                                           
                                                                                 
    uart_printf("IRQ SP: 0x%x\n", irq_sp);                                    
                                                                                 
    /* Dump IRQ stack and search for 0xCAFEBABE */                               
    asm volatile (                                                               
        "mov r1, %[irq_sp]\n"           /* Load IRQ stack pointer */          
        "mov r2, %[stack_ptr_irq]\n"    /* Load raw_stack_irq pointer */      
        "mov r3, #512\n"                /* Number of words to pop */          
        "ldr r12, =0xCAFEBABE\n"        /* Load marker to check */            
                                                                                 
        "2:\n"                                                                  
        "ldr r0, [r1], #4\n"            /* Load word from r1 and advance */  
        "str r0, [r2], #4\n"            /* Store word in raw_stack_irq */    
        "cmp r0, r12\n"                 /* Compare with marker */            
        "beq loop_forever_irq\n"        /* If found, loop forever */         
        "subs r3, r3, #1\n"                                                  
        "bne 2b\n"                      /* Loop until counter hits 0 */      
        "b continue_irq\n"              /* Continue if marker not found */   
                                                                                 
    "loop_forever_irq:\n"                                                     
        "b loop_forever_irq\n"          /* Infinite loop if marker found */  
                                                                                 
    "continue_irq:\n"                                                         
        :                                                                        
        : [stack_ptr_irq] "r" (stack_ptr_irq), [irq_sp] "r" (irq_sp)         
        : "r0", "r1", "r2", "r3", "r12", "memory"                    
    );                                                                           
                                                                                 
    /* Print IRQ stack in address:value format */                                
    for (i = 0; i < 512; i++) {                                                  
        uart_printf("Address: 0x%x, Value: 0x%x\n",                           
                    (unsigned int)&raw_stack_irq[i], raw_stack_irq[i]);          
    }                                                                            
                                                                                 
    uart_puts("End Sched Run\n");                                              
                                                                                 
                                                                                
}



void scheduler_run() {
    if (!scheduler_initialized) {
        uart_puts("uninitialized");
        return;
    }

    process_t *old_proc = current_process;
    process_t *new_proc = NULL;

    uart_puts("Called Sched Run\n");

    /* Pick next process to run */
    do {
        new_proc = scheduler.schedule_next();
    } while (new_proc->state == BLOCKED);

    /* Mark new process as running */
    new_proc->state = RUNNING;

    /* Save the current context (if old_proc is valid) */
    if (old_proc) {
        asm volatile (
            /* Save general-purpose registers r0-r12 */
            "stmia %[regs], {r0-r12}\n"
            /* Save stack pointer (sp) */
            "mov %[sp], sp\n"
            /* Save link register (lr) */
            "mov %[lr], lr\n"
            /* Save CPSR to r0 temporarily and store */
            "mrs r0, cpsr\n"
            "str r0, [%[cpsr]]\n"
            :
            : [regs] "r" (old_proc->registers),   /* Pointer to registers */
              [sp] "r" (&old_proc->stack_pointer), /* Pointer to sp */
              [lr] "r" (&old_proc->link_register), /* Pointer to lr */
              [cpsr] "r" (&old_proc->cpsr)        /* Pointer to cpsr */
            : "r0", "memory"
        );

        /* Mark old process as ready if it was running */
        if (old_proc->state == RUNNING) {
            old_proc->state = READY;
        }
    }

    uart_puts("Saved\n");
    /* Set current process */
    current_process = new_proc;

    /* Debugging print for context switch */
    uart_printf("Restoring CPSR: 0x%x\n", new_proc->cpsr);
    uart_printf("Switching to proc: SP = 0x%x, PC = 0x%x\n",
                new_proc->stack_pointer, new_proc->program_counter);

    /* Load the context of the new process */
    asm volatile (
        /* Load general-purpose registers r0-r12 */
        "ldmia %[regs], {r0-r12}\n"
        /* Load stack pointer (sp) */
        "ldr sp, [%[sp]]\n"
        /* Load link register (lr) */
        "ldr lr, [%[lr]]\n"
        /* Load CPSR and switch to new context */
        "ldr r0, [%[cpsr]]\n"
        "msr cpsr_c, r0\n"
        /* Load PC from program_counter (NOT from stack) */
        "ldr pc, [%[pc]]\n"
        :
        : [regs] "r" (new_proc->registers),
          [sp] "r" (&new_proc->stack_pointer),
          [lr] "r" (&new_proc->link_register),
          [cpsr] "r" (&new_proc->cpsr),
          [pc] "r" (&new_proc->program_counter)
        : "r0", "memory"
    );

    /* Should never get here if context switch worked */
    uart_puts("Return Proc\n");

    /* Halt system if something went wrong */
    while (1) {
        uart_puts("Infinite loop trap\n");
    }
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

    /* Ensure paddr is aligned to 4 KB boundary */
    if (paddr & 0xFFF) {
        uart_puts("Error: paddr is not 4 KB aligned!\n");
        return NULL;
    }

    /* Cast the allocated frame as a process_t pointer */
    proc = (process_t *)paddr;

    /* Zero out the entire segment (1MB region) */
    uint32_t *ptr = (uint32_t *)paddr;
    for (i = 0; i < SEGMENT_SIZE / sizeof(uint32_t); i++) {
        ptr[i] = 0;
    }

    uart_printf("Allocated frame at 0x%x\n", paddr);

    /* Copy the entry point function into process memory */
    asm volatile (
        "mov r0, %[src]\n"
        "mov r1, %[dst]\n"
        "mov r2, %[size]\n"
        "1:\n"
        "ldr r3, [r0], #4\n"
        "str r3, [r1], #4\n"
        "subs r2, r2, #4\n"
        "bne 1b\n"
        :
        : [src] "r" (entry_point),
          [dst] "r" (paddr),
          [size] "r" (PROCESS_CODE_SIZE)
        : "r0", "r1", "r2", "r3", "memory"
    );

    /* Set entry point to copied function */
    entry_point = (void (*)())paddr;

    /* Debug check to verify copied function */
    uart_printf("Checking Copied Code: dst[0] = 0x%x, dst[1] = 0x%x\n",
                ((uint32_t *)paddr)[0], ((uint32_t *)paddr)[1]);

    /* Set segment base to the allocated frame address */
    proc->segment_base = paddr;

    /* Set up user stack */
    proc->stack_pointer = paddr + USER_STACK_TOP;
    uint32_t *stack = (uint32_t *)proc->stack_pointer;

    /* Align stack to 8 bytes for safety */
    stack = (uint32_t *)((uint32_t)stack & ~7);

    /* Set initial context: SPSR, PC, LR */
    *(--stack) = 0x600001D3;             /* SPSR with correct mode */
    *(--stack) = (uint32_t)entry_point;  /* PC (entry point in copied process memory) */
    *(--stack) = (uint32_t)scheduler_run; /* LR fallback if process exits */

    /* Clear R0-R12 */
    for (i = 0; i <= 12; i++) {
        *(--stack) = 0;
    }

    /* Set SP correctly */
    proc->stack_pointer = (uint32_t)stack;

    /* Debug Stack Verification */
    uart_printf("Stack Check: sp[0] = 0x%x, sp[1] = 0x%x, sp[2] = 0x%x\n",
                stack[0], stack[1], stack[2]);

    /* Set link register to a safe exit handler */
    proc->link_register = (uint32_t)scheduler_run;

    /* Set initial CPSR to System mode with IRQs enabled */
    proc->cpsr = 0x1D3;

    /* Set program counter to copied function */
    proc->program_counter = paddr;

    /* Set process state and mode */
    proc->current_mode = 1;  /* User mode for the process */
    proc->state = READY;

    /* Print stack contents for verification */
    uart_printf("Final Stack: sp[0] = 0x%x, sp[1] = 0x%x, sp[2] = 0x%x\n",
                ((uint32_t *)proc->stack_pointer)[0],
                ((uint32_t *)proc->stack_pointer)[1],
                ((uint32_t *)proc->stack_pointer)[2]);

    /* Debug print to confirm process creation */
    uart_puts("Process created.\n");
    uart_printf("New Process SP: 0x%x, PC: 0x%x\n", proc->stack_pointer, proc->program_counter);

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
