#ifndef SCHED_H
#define SCHED_H

#include <types.h> 
#include <ddr.h>
#include <uart.h>
#include <utils.h>

#define SEGMENT_SIZE        0x100000   // 1MB per process
#define USER_STACK_SIZE     (64 * 1024) // 200 KB User Stack

#define HEAP_BASE_OFFSET    0xC8000   // Heap base remains at 800 KB
#define PROCESS_CODE_SIZE (600 * 1024)  /* 600 KB */

#define MAX_PROCESSES 32

#define MODE_Usr 		0x10	/* thread-mode, unprivileged */
#define MODE_FIQ 		0x11	/* FIQ-mode (always privileged) */
#define MODE_IRQ 		0x12	/* IRQ-mode (always privileged) */
#define MODE_Supervisor 0x13	/* SVC-mode (always privileged) */
#define MODE_Abort 		0x17	/* Abort-mode (always privileged) */
#define MODE_Undef	 	0x1B	/* Undefined-mode (always privileged) */
#define MODE_System 	0x1F	/* thread-mode, privileged */
#define I_F_BIT  		0xC0	/* I and F bits for CPSR register */

typedef enum {
    UNDEFINED = 0,
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} process_state_t;

typedef struct{
    uint32_t registers[13];  
    uint32_t stack_pointer;  
    uint32_t link_register;  
    uint32_t cpsr;
    
    uint32_t program_counter; /* Program counter (entry point) */
    uint32_t segment_base; /* location of process */
    uint32_t current_mode;
    uint32_t shared_page;
    char stack[USER_STACK_SIZE]; /* User stack. Used because we are
    turning off the MMU */
    uint32_t stack_top; /* Top of the stack */
    char name[16]; /* Process name for debugging */
    uint32_t pid;
    uint32_t priority;
    process_state_t state;
} process_t;

typedef struct {
    process_t processes[MAX_PROCESSES];
    process_t *idle_process;
    
    int current_index;
    int num_processes;

    //process_t* (*schedule_next)(); 
} scheduler_t;


void scheduler_tick(void);

process_t* process_create(void (*entry_point)(void));
void scheduler_init();
void scheduler_run();

void yield(void);


#endif // SCHED_H
