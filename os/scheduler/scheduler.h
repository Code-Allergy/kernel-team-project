#ifndef SCHED_H
#define SCHED_H

#include <types.h> 
#include <ddr.h>
#include <uart.h>
#include <utils.h>

#define SEGMENT_SIZE        0x100000   // 1MB per process
#define USER_STACK_SIZE     (200 * 1024) // 200 KB User Stack

#define HEAP_BASE_OFFSET    0xC8000   // Heap base remains at 800 KB
#define PROCESS_CODE_SIZE (600 * 1024)  /* 600 KB */






#define MAX_PROCESSES 32

typedef enum {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} process_state_t;



typedef struct{
    uint32_t pid;
    process_state_t state;
    uint32_t priority;
    
    


    uint32_t registers[13];  
    uint32_t stack_pointer;  
    uint32_t link_register;  
    uint32_t cpsr;


    uint32_t program_counter; /* Program counter (entry point) */
    uint32_t segment_base; /* location of process */
    uint32_t current_mode;
    uint32_t shared_page;
} process_t;

typedef struct {
    process_t *processes[MAX_PROCESSES];
    process_t idle_process;
    
    int current_index;
    int num_processes;

    process_t* (*schedule_next)(); 
} scheduler_t;


void scheduler_tick(void);


process_t* process_create(void (*entry_point)(void));
void scheduler_init();
int scheduler_add_process(process_t *proc);
int context_switch(process_t *old_proc, process_t *new_proc);
void boot_test();
process_t* round_robin_scheduler();
void scheduler_run();
void restore_context(process_t *proc);





#endif // SCHED_H
