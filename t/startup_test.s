#include <syscall.h>

.section .text, "ax", @progbits

.global _start
_start:
    mov $0, %rdi
    mov $__NR_exit, %rax
    syscall
