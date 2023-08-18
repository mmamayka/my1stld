
#include <syscall.h>

.section .text, "ax", @progbits

.global exit
exit:
    mov $__NR_exit, %rax
    syscall


