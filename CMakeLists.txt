cmake_minimum_required(VERSION 3.15)
project(m C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -------------------------
# lib
# -------------------------
add_library(toolbox STATIC
        modules/m/m.c
        modules/mtest/mtest.c
        modules/mbuf/mbuf.c
)

target_include_directories(toolbox PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/modules
)

# -------------------------
# testing
# -------------------------
enable_testing()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/test_*.c)

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_SOURCE})

    target_link_libraries(${TEST_NAME} PRIVATE toolbox)

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()