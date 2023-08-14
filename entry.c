#include <stdint.h>
#include "printf.h"

// __attribute__((noreturn))
void entry(uint64_t* process_context) {
    printf("m1stld started successfully!\n");
}
