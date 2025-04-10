#include <syscall.h>
#include <errno.h>
#include <gpio.h>

typedef int (*syscall_func)(va_list args);
syscall_func syscall_vector_table[NUM_SYSCALLS];
#define __SYSCALL__(n, swi_routine) syscall_vector_table[n] = swi_routine

void setup_syscall_table(void)
{
    __SYSCALL__(SYS_YIELD, sys_default);
    __SYSCALL__(SYS_READ, sys_read);
    __SYSCALL__(SYS_WRITE, sys_write);
}

int handle_syscall(uint16_t syscall_num, ...)
{
    va_list args;
    int result;

    GPIO_set(GPIO1_BASE, LED2);
    if (syscall_num > NUM_SYSCALLS)
    {
        return (-ENOSYSCALL);
    }

    if (syscall_vector_table[syscall_num])
    {
        va_start(args, syscall_num);
        result = syscall_vector_table[syscall_num](args);
        return result;
    }
    return (-ENOSYSCALL);
}

int sys_default(va_list args) { return 0; }

int sys_read(va_list args)
{
    uint8_t fd;
    char* buffer;
    uint32_t len;

    fd     = (uint8_t) va_arg(args, int);
    buffer = va_arg(args, char*);
    len    = (uint32_t) va_arg(args, int);
    va_end(args);

    GPIO_set(GPIO1_BASE, LED3);
    return 0;
}
int sys_write(va_list args)
{
    uint8_t fd;
    char* buffer;
    uint32_t len;

    fd     = (uint8_t) va_arg(args, int);
    buffer = va_arg(args, char*);
    len    = (uint32_t) va_arg(args, int);

    return 0;
}
