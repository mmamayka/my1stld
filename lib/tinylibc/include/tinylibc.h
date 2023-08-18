#ifndef MY1STLD_LIB_TINYLIBC_H
#define MY1STLD_LIB_TINYLIBC_H

#include "printf.h"

void exit(int exit_code);

#define ASSERT_EXIT_CODE 666
#define assert(condition) \
    do { \
        if(!condition) { \
            printf("assetion %s failed at %s:%i\n", \
                #condition, __FILE__, __LINE__); \
            exit(ASSERT_EXIT_CODE); \
        } \
    } while(0) \

#endif /* MY1STLD_LIB_TINYLIBC_H */
