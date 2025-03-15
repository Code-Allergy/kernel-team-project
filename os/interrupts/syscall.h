#ifndef __SYSCALL_H
#define __SYSCALL_H

#include <types.h>

#define __SYSCALL__(n, swi_routine) \
                                    syscall_vector_table[n] = swi_routine \

#define SYSCALL_ARGS1(type0, arg0)  \
    do{ \
        arg0 = va_arg(args, type0); \
        va_end(args); \
    }while(0)

#define SYSCALL_ARGS2(type0, arg0, type1, arg1)  \
    do{ \
        arg0 = va_arg(args, type0); \
        arg1 = va_arg(args, type1); \
        va_end(args); \
    }while(0)

#define SYSCALL_ARGS3(type0, arg0, type1, arg1, type2, arg2)  \
    do{ \
        arg0 = va_arg(args, type0); \
        arg1 = va_arg(args, type1); \
        arg2 = va_arg(args, type2); \
        va_end(args); \
    }while(0)

#define SYSCALL_ARGS4(type0, arg0, type1, arg1, type2, arg2, type3, arg3)  \
    do{ \
        arg0 = va_arg(args, type0); \
        arg1 = va_arg(args, type1); \
        arg2 = va_arg(args, type2); \
        arg3 = va_arg(args, type3); \
        va_end(args); \
    }while(0)

#define SYSCALL_ARGS5(type0, arg0, type1, arg1, type2, arg2, type3, arg3, type4, arg4)  \
    do{ \
        arg0 = va_arg(args, type0); \
        arg1 = va_arg(args, type1); \
        arg2 = va_arg(args, type2); \
        arg3 = va_arg(args, type3); \
        arg4 = va_arg(args, type4); \
        va_end(args); \
    }while(0)

#define SYSCALL_ARGS6(type0, arg0, type1, arg1, type2, arg2, type3, arg3, type4, arg4, type5, arg5)  \
    do{ \
        arg0 = va_arg(args, type0); \
        arg1 = va_arg(args, type1); \
        arg2 = va_arg(args, type2); \
        arg3 = va_arg(args, type3); \
        arg4 = va_arg(args, type4); \
        arg5 = va_arg(args, type5); \
        va_end(args); \
    }while(0)

#define SYS_DEFAULT         0
#define SYS_READ            1
#define SYS_WRITE           3
/* Define more swi*/

/* Number of SWIs is the last SWI NUM +1 */
#define NUM_SYSCALLS        (SYS_WRITE + 1)

/* Declare syscall routines */
int sys_default(va_list args);
int sys_read(va_list args);
int sys_write(va_list args);


typedef int (*syscall_func)(va_list args);
syscall_func syscall_vector_table[NUM_SYSCALLS];
/* Register routine in syscall table */
void setup_syscall_table(void){
__SYSCALL__(SYS_DEFAULT, sys_default);
__SYSCALL__(SYS_READ, sys_read);
__SYSCALL__(SYS_WRITE, sys_write);
}

int handle_syscall(int syscall_num, ...);

#endif  /*__SYSCALL_H */