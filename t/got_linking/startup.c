#include "lib.h"
#include "tinylibc.h"

void _start() {
    assert(so_variable == 1);
    exit(0);
}
