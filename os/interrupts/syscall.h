#ifndef __SYSCALL_H
#define __SYSCALL_H

#include <types.h>

#define SYS_YIELD 0
#define SYS_READ  1
#define SYS_WRITE 3
/* Define more swi*/

/* Number of SWIs is the last SWI NUM +1 */
#define NUM_SYSCALLS (SYS_WRITE + 1)

/* Declare syscall routines */
int sys_default(va_list args);
int sys_read(va_list args);
int sys_write(va_list args);

/* Register routine in syscall table */
/* Populate this in syscall.c */
void setup_syscall_table(void);

extern int syscall(uint16_t syscall_num, ...);
int handle_syscall(uint16_t syscall_num, ...);

#endif /*__SYSCALL_H */