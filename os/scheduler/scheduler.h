#ifndef SCHED_H
#define SCHED_H

#include <types.h> 
#include <ddr.h>
#include <uart.h>
#include <utils.h>

#define SEGMENT_SIZE        0x100000   // 1MB per process
#define USER_STACK_SIZE     0x19000    // 100 KB User Stack
#define KERNEL_STACK_SIZE   0x19000    // 100 KB Kernel Stack

#define USER_STACK_TOP      SEGMENT_SIZE        // User stack starts at top of segment
#define KERNEL_STACK_TOP    (USER_STACK_TOP - USER_STACK_SIZE) // Kernel stack below user stack
#define HEAP_BASE_OFFSET    0xC8000   // Heap base remains at 800 KB






#define MAX_PROCESSES 32

typedef enum {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} process_state_t;



typedef struct {
    uint32_t pid;
    process_state_t state;
    uint32_t priority;
    
    


    uint32_t registers[13];  
    uint32_t stack_pointer;  
    uint32_t link_register;  
    uint32_t cpsr;

    uint32_t segment_base; /* location of process */
    uint32_t current_mode;
} process_t;

typedef struct {
    process_t *processes[MAX_PROCESSES];
    process_t idle_process;
    
    int current_index;
    int num_processes;

    process_t* (*schedule_next)(); 
} scheduler_t;



process_t* process_create(void (*entry_point)(void));
void scheduler_init();
int scheduler_add_process(process_t *proc);
int context_switch(process_t *old_proc, process_t *new_proc);
void boot_test();
process_t* round_robin_scheduler();
void scheduler_run();
#endif // SCHED_H
