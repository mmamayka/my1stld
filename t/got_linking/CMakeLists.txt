cmake_minimum_required(VERSION 3.22.1)

add_library(got_linking_lib SHARED)

target_compile_options(got_linking_lib PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${TEST_CFLAGS}>
    $<$<COMPILE_LANGUAGE:ASM>:${TEST_ASMFLAGS}>
)

target_link_options(got_linking_lib PRIVATE
    -nostartfiles -nodefaultlibs
)

target_sources(got_linking_lib PRIVATE lib.c)



add_executable(got_linking_test)

#target_include_directories(got_linking_test PRIVATE ../../lib/tinylibc/include)

target_compile_options(got_linking_test PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${TEST_CFLAGS}>
    $<$<COMPILE_LANGUAGE:ASM>:${TEST_ASMFLAGS}>
)

target_link_options(got_linking_test PRIVATE
    ${TEST_LDFLAGS}
)

target_link_libraries(got_linking_test PRIVATE got_linking_lib tinylibc)

target_sources(got_linking_test PRIVATE startup.c)

add_test(NAME got_linking_test COMMAND got_linking_test)
