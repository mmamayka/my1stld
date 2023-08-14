
AS := gcc -c
CC := gcc
LD := gcc

ASFLAGS := -x assembler-with-cpp
CFLAGS := -Wall -Wextra -O2 -g
LDFLAGS := -nostdlib -nodefaultlibs

.PHONY: run
run: test
	./test

ld-dynamic.so: startup.o entry.o
	$(LD) -o $@ $^ -Wl,--entry=startup -nostdlib -nodefaultlibs \
		-Wl,--no-undefined -fPIC
	strip --remove-section=.interp $@

test: test.o ld-dynamic.so
	$(LD) -o $@ $< -Wl,--dynamic-linker=$(CURDIR)/ld-dynamic.so -nostdlib
