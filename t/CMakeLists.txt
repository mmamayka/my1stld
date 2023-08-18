cmake_minimum_required(VERSION 3.22.1)

set(TEST_CFLAGS -O2 -Wall -Wextra -g -fno-stack-protector)
set(TEST_LDFLAGS -nostartfiles -nodefaultlibs 
    -Wl,--dynamic-linker=${CMAKE_BINARY_DIR}/${DYNAMIC_LINKER_SO}
    -Wl,-hash-style=sysv
)
set(TEST_ASMFLAGS -x assembler-with-cpp)

# startup test

add_executable(startup_test)

target_compile_options(startup_test PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${TEST_CFLAGS}>
    $<$<COMPILE_LANGUAGE:ASM>:${TEST_ASMFLAGS}>
)

target_link_options(startup_test PRIVATE ${TEST_LDFLAGS})

target_sources(startup_test PRIVATE startup_test.s)

add_test(NAME startup_test COMMAND startup_test)

# got linking test

add_subdirectory(got_linking)
