cmake_minimum_required(VERSION 3.22.1)

set(TINYLIBC_CFLAGS -O2 -Wall -Wextra -g -fno-stack-protector)
set(TINYLIBC_ASFLAGS -x assembler-with-cpp ${TINYLIBC_CFLAGS})

add_library(tinylibc STATIC)

target_compile_options(tinylibc PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${TINYLIBC_CFLAGS}>
    $<$<COMPILE_LANGUAGE:ASM>:${TINYLIBC_ASFLAGS}>
)

target_include_directories(tinylibc PUBLIC include)

file(GLOB TINYLIBC_SRC "./*.[cs]")

target_sources(tinylibc PRIVATE ${TINYLIBC_SRC})
