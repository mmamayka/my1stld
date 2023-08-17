cmake_minimum_required(VERSION 3.22.1)

project(dynamic-linker LANGUAGES C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DYNAMIC_LINKER_SO ld-dynamic.so)
set(DYNAMIC_LINKER_CFLAGS -O2 -Wall -Wextra -g -fno-stack-protector)
set(DYNAMIC_LINKER_ASMFLAGS -x assembler-with-cpp)
set(DYNAMIC_LINKER_LDFLAGS -nostartfiles -nodefaultlibs -Wl,--entry=startup
    -Wl,--no-undefined -static-pie)
set(DYNAMIC_LINKER_SRC_PATH src)
set(DYNAMIC_LINKER_INC_PATH ${DYNAMIC_LINKER_SRC_PATH}/include)

file(GLOB DYNAMIC_LINKER_SRC "${DYNAMIC_LINKER_SRC_PATH}/*.[cs]")

add_library(${DYNAMIC_LINKER_SO} SHARED)

set_target_properties(${DYNAMIC_LINKER_SO} PROPERTIES PREFIX "" SUFFIX "")

target_compile_options(${DYNAMIC_LINKER_SO} PRIVATE 
    $<$<COMPILE_LANGUAGE:C>:${DYNAMIC_LINKER_CFLAGS}>
    $<$<COMPILE_LANGUAGE:ASM>:${DYNAMIC_LINKER_ASMFLAGS}>
)

target_link_options(${DYNAMIC_LINKER_SO} PRIVATE ${DYNAMIC_LINKER_LDFLAGS})

target_include_directories(${DYNAMIC_LINKER_SO} 
    PRIVATE ${DYNAMIC_LINKER_INC_PATH})

target_sources(${DYNAMIC_LINKER_SO} PRIVATE ${DYNAMIC_LINKER_SRC})



enable_testing()

add_subdirectory(t)
