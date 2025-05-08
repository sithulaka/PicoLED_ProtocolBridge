cmake_minimum_required(VERSION 3.13)

# Include guard
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed")
endif()

# Initialize pico-sdk from submodule
include(pico_sdk_import.cmake)

# Project declaration
project(led_project 
    VERSION 1.0.0
    DESCRIPTION "WS2812 LED Control Project for Raspberry Pi Pico"
    LANGUAGES C CXX ASM
)

# Compiler standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)  # Changed from 17 to 20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Compiler warnings
# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Werror
    )
endif()

# Output directories setup
set(PROJECT_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/output")
file(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR})

# Initialize the Pico SDK
pico_sdk_init()

# Source files
file(GLOB_RECURSE PROJECT_SOURCES 
    "src/*.cpp"
    "src/*.c"
)

# Header files
file(GLOB_RECURSE PROJECT_HEADERS 
    "include/*.h"
    "include/*.hpp"
)

# Create the executable
add_executable(${PROJECT_NAME} 
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

# Generate PIO header file
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Library dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
    hardware_pio
)

# USB and UART configuration
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Generate additional output files
pico_add_extra_outputs(${PROJECT_NAME})

# Post-build copy command
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.uf2
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.bin
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.hex
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.dis
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.elf.map
        ${PROJECT_OUTPUT_DIR}/
)