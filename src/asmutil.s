

#include <asm/unistd.h>

.section .text, "ax", @progbits

.global exit
exit:
    mov $1, %rdi
    syscall

.global _putchar
_putchar:
    push %rdi

    mov $1, %rdi
    mov %rsp, %rsi
    mov $1, %rdx
    mov $__NR_write, %rax
    syscall

    pop %rdi

    ret
