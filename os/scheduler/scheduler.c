#include <scheduler.h>
#include <types.h>
#include <ddr.h>
#include <uart.h>
#include <utils.h>
#include <timer.h>
#include <mmu.h>
#include <circular_buffer.h>
#include <syscall.h>
#include <motor.h>
#include <driver_defs.h>

static scheduler_t scheduler;
static generic_circular_buffer_t processes;
static generic_circular_buffer_t free_processes;
static generic_circular_buffer_t ready_queue;
process_t* current_process = NULL;

volatile uint32_t p1_heartbeat = 0, p2_heartbeat = 0, idle_proc_heartbeat = 0;

void idle_task()
{
    volatile int idle_count = 0;
    uart_printf("Idle task running...\n");
    while (1)
    {
        uart_printf("Idle task heartbeat: %d\n", idle_proc_heartbeat);
        idle_count = 0xFFFF;
        while (idle_count > 0)
        {
            idle_count--;
        }
        idle_proc_heartbeat++;
        yield();
    }
}

void P1()
{
    volatile int count     = 0;
    volatile uint32_t cpsr = 0;
    uart_printf("P1 running...\n");
    while (1)
    {
        // __asm__ volatile ("mrs %0, cpsr" : "=r"(cpsr));
        // uart_printf("P2: cpsr: 0x%x\n", cpsr);
        uart_printf("P1: heartbeat: %d\n", p1_heartbeat);
        count = 0xFF;
        while (count > 0)
        {
            count--;
        }
        p1_heartbeat++;
        yield();
    }
}

void P2()
{
    volatile int count     = 0;
    volatile uint32_t cpsr = 0;
    uart_printf("P2 running...\n");
    while (1)
    {
        // __asm__ volatile ("mrs %0, cpsr" : "=r"(cpsr));
        // uart_printf("P2: cpsr: 0x%x\n", cpsr);
        uart_printf("P2: heartbeat: %d\n", p2_heartbeat);
        count = 0xFF;
        while (count > 0)
        {
            count--;
        }
        p2_heartbeat++;
        yield();
    }
}

/* Moter driver process */
void motor_task()
{
    char uart_buffer[10];
    int x_joystick, y_joystick, read = 0;

    uart_printf("Motor task running...\n");
    motor_init();
    uart_printf("Motor task: Motors initialized\n");

    while (1)
    {
        read = uart_readline(1, uart_buffer, 100);
        if (read == 5) /* xayb*/
        {
            x_joystick = (int) uart_buffer[1] - 128;
            y_joystick = (int) uart_buffer[3] - 128;
            uart_printf("Received: x:%d, y:%d\n", x_joystick, y_joystick);

            if (x_joystick > -5 && x_joystick < 5 && y_joystick > 5)
            { /* straight forward*/
                motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_FORWARD);
            }
            else if (x_joystick > -5 && x_joystick < 5 && y_joystick < -5)
            { /* straight backward*/
                motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_BACKWARD);
            }
            else if (x_joystick > 5 && y_joystick > 5)
            { /* right forward*/
                motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_FORWARD | MOTOR_DIR_RIGHT);
            }
            else if (x_joystick > 5 && y_joystick < -5)
            { /* right backward*/
                motor_ioctl(MOTOR_SET_DIR,
                            MOTOR_DIR_BACKWARD | MOTOR_DIR_RIGHT);
            }
            else if (x_joystick < -5 && y_joystick > 5)
            { /* left forward*/
                motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_FORWARD | MOTOR_DIR_LEFT);
            }
            else if (x_joystick < -5 && y_joystick < -5)
            { /* left backward*/
                motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_BACKWARD | MOTOR_DIR_LEFT);
            }
            else if ((x_joystick > -5 && x_joystick < 5) &&
                     (y_joystick > -5 && y_joystick < 5))
            { /* stop*/
                motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_STOP);
            }
            else
            {
                /* stop */
                motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_STOP);
            }
        }
        uart_printf("Motor task: yielding...\n");
        yield();
    }
}

volatile uint32_t timer_tick = 0;
void scheduler_tick()
{
    uart_printf("TICK: %u\n", timer_tick);
    timer_tick++;
    // scheduler_should_switch = 1;
}

void snprintf(char* dest, const char* src, int n)
{
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

void scheduler_init()
{
    int i;
    process_t *idle_proc, *proc;

    check_mode();

    /* Initialize scheduler state */
    scheduler.current_index = 0;
    scheduler.num_processes = 0;
    // scheduler_should_switch = 1;

    /* Initialize the circular buffer for processes */
    generic_circular_buffer_init(&processes);
    generic_circular_buffer_init(&free_processes);
    generic_circular_buffer_init(&ready_queue);

    /* Populate free processes ring buffer */
    for (i = 0; i < MAX_PROCESSES; i++)
    {
        proc                  = &scheduler.processes[i];
        proc->pid             = i;
        proc->state           = UNDEFINED;
        proc->priority        = 0;
        proc->program_counter = 0;
        proc->stack_pointer   = 0;
        proc->link_register   = 0;
        proc->cpsr            = 0;
        generic_circular_buffer_push(&free_processes, (void*) (proc));
    }

    /* Create idle process */
    uart_puts("Creating idle process...\n");
    idle_proc = process_create(idle_task);
    if (!idle_proc)
    {
        uart_puts("Failed to create idle process\n");
        while (1)
            ;
    }
    snprintf(idle_proc->name, "Idle Process", sizeof(idle_proc->name));
    uart_printf("Created process: %s\n", idle_proc->name);

    generic_circular_buffer_push(&processes, (void*) idle_proc);
    scheduler.idle_process = idle_proc;

    /* create P1 and P2 */
    uart_puts("Creating P1 process...\n");
    proc = process_create(P1);
    if (!proc)
    {
        uart_puts("Failed to create P1 process\n");
        while (1)
            ;
    }
    snprintf(proc->name, "P1", sizeof(proc->name));
    uart_printf("Created process: %s\n", proc->name);
    generic_circular_buffer_push(&processes, (void*) proc);
    generic_circular_buffer_push(&ready_queue, (void*) proc);
    scheduler.num_processes++;
    uart_puts("Creating P2 process...\n");
    proc = process_create(P2);
    if (!proc)
    {
        uart_puts("Failed to create P2 process\n");
        while (1)
            ;
    }
    snprintf(proc->name, "P2", sizeof(proc->name));
    uart_printf("Created process: %s\n", proc->name);
    generic_circular_buffer_push(&processes, (void*) proc);
    generic_circular_buffer_push(&ready_queue, (void*) proc);
    scheduler.num_processes++;

    /* create motor process */
    uart_puts("Creating motor process...\n");
    proc = process_create(motor_task);
    if (!proc)
    {
        uart_puts("Failed to create motor process\n");
        while (1)
            ;
    }
    snprintf(proc->name, "Motor Process", sizeof(proc->name));
    uart_printf("Created process: %s\n", proc->name);
    generic_circular_buffer_push(&processes, (void*) proc);
    generic_circular_buffer_push(&ready_queue, (void*) proc);
    scheduler.num_processes++;

    /* Use Timer 2 for scheduling */
    /* 100ms quantum */
    timer_init(TIMER2, 100, scheduler_tick);
    // timer_start(TIMER2); (MMU does not like the motor task)
    uart_puts("Timer started\n");

    uart_puts("Scheduler Initialized\n");
}

void scheduler_run()
{
    process_t* next_proc;
    bool status;

    // uart_puts("Scheduler running...\n");

    if (current_process != NULL && current_process != scheduler.idle_process)
    {
        current_process->state = READY;
        // uart_printf("Scheduler: process %s added to ready queue\n",
        // current_process->name);
        generic_circular_buffer_push(&ready_queue, (void*) current_process);
    }

    status = generic_circular_buffer_pop(&ready_queue, (void**) &next_proc);
    if (!status)
    {
        // uart_puts("No READY process. Running idle...\n");
        next_proc = scheduler.idle_process;
    }

    current_process        = next_proc;
    current_process->state = RUNNING;
    // uart_printf("Scheduler: starting proc: %s, pc: %x \n",
    // current_process->name, current_process->program_counter);
    restore_context_asm(); // Jump to the process
    uart_printf("Ummm... we should not be here\n");
    while (1)
        ;

    __builtin_unreachable();
}

process_t* process_create(void (*entry_point)(void))
{
    int i;
    process_t* proc;
    bool status;

    status = generic_circular_buffer_pop(&free_processes, (void**) &proc);
    if (!status)
    {
        uart_puts("No free process slots available\n");
        return NULL;
    }
    proc->state           = NEW;
    proc->priority        = 1; // Default priority, can be modified
    proc->program_counter = (uint32_t) entry_point;
    proc->segment_base    = (uint32_t) proc;
    /* processes will be run in system mode since
        we are currently having issues context switching betweeen
        system and user mode with the MMU*/
    proc->current_mode    = MODE_System | I_F_BIT;
    proc->cpsr            = MODE_System | I_F_BIT;
    proc->shared_page     = 0;

    /*  Clear registers */
    for (i = 0; i < 16; i++)
    {
        proc->registers[i] = i;
    }

    proc->link_register = 0;

    /* Setup the user stack */
    proc->stack_top     = (uint32_t) (proc->stack + USER_STACK_SIZE);
    proc->stack_pointer = proc->stack_top;

    // proc->state = READY;
    scheduler.num_processes++;

    /* proc->pid set already in scheduler_init() */

    return proc;
}

void yield()
{
    uart_puts("Yielding...\n");
    syscall(SYS_YIELD, 0);
}

void uart_print_hex(uint32_t value)
{
    char hex_string[9]; /* 8 characters + null terminator */
    int i;

    hex_string[8] = '\0'; /* Null-terminate the string */

    for (i = 7; i >= 0; i--)
    {
        int digit     = value & 0xF; /* Extract the last 4 bits */
        hex_string[i] = (digit < 10) ? ('0' + digit) : ('A' + (digit - 10));
        value >>= 4; /* Shift right by 4 bits */
    }

    uart_puts("0x");
    uart_puts(hex_string);
}
