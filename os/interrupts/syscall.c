#include <syscall.h>
#include <errno.h>

int handle_syscall(uint16_t syscall_num, ...){
    va_list args;
    int result;

    if (syscall_num > NUM_SYSCALLS){
        return (-ENOSYSCALL);
    }

    if (syscall_vector_table[syscall_num]) {
        va_start(args, syscall_num);
        result = syscall_vector_table[syscall_num](args);
        return result;
    }
    return (-ENOSYSCALL);
}


int sys_default(va_list args){
    return 0;
}

int sys_read(va_list args){
    uint8_t fd;
    char* buffer;
    int16_t len;
    SYSCALL_ARGS3(uint8_t, fd, char*, buffer, int16_t, len);

    return 0;
}
int sys_write(va_list args){
    uint8_t fd;
    char* buffer;
    int16_t len;
    SYSCALL_ARGS3(uint8_t, fd, char*, buffer, int16_t, len);

    return 0;
}
