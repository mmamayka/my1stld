#if !defined(__linux__) || !defined(__x86_64__)
#   error "Unsupported OS or architecture"
#endif

#include <asm/unistd.h> // __NR_exit

.section .text, "ax", @progbits
.global _putchar
_putchar:
    push %rdi

    mov %rsp, %rsi
    mov $1, %rdi
    mov $1, %rdx
    mov $__NR_write, %rax
    syscall

    pop %rdi

    ret

.global exit
exit:
    mov $__NR_exit, %rax
    syscall
