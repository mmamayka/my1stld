#include <syscall.h>

.section .text, "ax", @progbits

.global _start
_start:
    mov $1, %rdi
    mov $__NR_exit, %rax
    syscall
