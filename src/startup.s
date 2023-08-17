

// only System V ABI & x86-64 arch supported 
#if !defined(__linux__) || !defined(__x86_64__)
#   error "Unsupported OS or architecture"
#endif

#include <asm/unistd.h> // __NR_exit

.section .text, "ax", @progbits

.global entry

.global startup
startup:
    // we need to clear the rbp register according to System V ABI
    xor %rbp, %rbp

    // the rsp register contains a pointer to the process context prepared
    // by linux, we want to pass it as the 1st argument of main function
    mov %rsp, %rdi

    // minimal initialization is finished, so let's go to the C-code!
    call entry

    // main is noreturn function, so we exit with a error in case the
    // control flow returned here
    mov $1, %rdi
    mov $__NR_exit, %rax
    syscall

