#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define sys_print_int     1
#define sys_print_string  4
#define sys_read_int      5
#define sys_read_string   8
#define sys_print_char   11
#define sys_read_char    12
#define sys_exit         10
#define sys_exit2        17

#define SYSCALL(syscall_type) \
"  li $v0, " STR(syscall_type) "\n" \
"  syscall\n"

#endif //__SYSCALL_H__