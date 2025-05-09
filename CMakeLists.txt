cmake_minimum_required(VERSION 3.13)
# Include guard
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed")
endif()
# Initialize pico-sdk from submodule
include(pico_sdk_import.cmake)
# Project declaration
project(PicoLED_ProtocolBridge
    VERSION 1.0.0
    DESCRIPTION "Combined WS2812 LED Control and DMX Protocol Bridge for Raspberry Pi Pico"
    LANGUAGES C CXX ASM
)
# Compiler standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PICO_PLATFORM rp2040)
set(PICO_COMPILER arm-none-eabi-gcc)
# Build type if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
# Compiler warnings and optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wno-unused-parameter
        -O3
    )
endif()
# Add this after your compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Disable missing-field-initializers warning for Pico-DMX files
    set_source_files_properties(
        Pico-DMX/src/DmxInput.cpp
        Pico-DMX/src/DmxOutput.cpp
        PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers"
    )
endif()

# Initialize the Pico SDK
pico_sdk_init()

# Include Pico-DMX library
include(${CMAKE_CURRENT_SOURCE_DIR}/Pico-DMX/interfaceLibForPicoSDK.cmake)

# Source files (using GLOB but with existence checks)
file(GLOB_RECURSE PROJECT_SOURCES
    "src/*.cpp"
    "src/*.c"
)
# Header files
file(GLOB_RECURSE PROJECT_HEADERS
    "include/*.h"
    "include/*.hpp"
)
# Verify we found sources
if(NOT PROJECT_SOURCES)
    message(FATAL_ERROR "No source files found in src/ directory")
endif()
# Create the executable
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)
# Generate PIO header files
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/extras/ws2812.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/Pico-DMX/extras/DmxInput.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/Pico-DMX/extras/DmxOutput.pio)
# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Pico-DMX/src
)
# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
    hardware_pio
    pico_multicore
    hardware_dma
    hardware_irq
    hardware_uart
    picodmx
)
# USB and UART configuration
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Generate additional output files
pico_add_extra_outputs(${PROJECT_NAME})

# Create an output directory in the project root
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/outputs)

# Add a custom command to copy all output files correctly
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying build artifacts to outputs directory..."
    
    # First verify these files exist before copying
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.elf
        ${CMAKE_SOURCE_DIR}/outputs/

    # Copy the other files with error handling for each
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2
        ${CMAKE_SOURCE_DIR}/outputs/
        || echo "Warning: UF2 file not found"
        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
        ${CMAKE_SOURCE_DIR}/outputs/
        || echo "Warning: BIN file not found"
        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
        ${CMAKE_SOURCE_DIR}/outputs/
        || echo "Warning: HEX file not found"
        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dis
        ${CMAKE_SOURCE_DIR}/outputs/
        || echo "Warning: DIS file not found"
        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${PROJECT_NAME}>.map
        ${CMAKE_SOURCE_DIR}/outputs/
        || echo "Warning: MAP file not found"
        
    COMMENT "Copying build artifacts to outputs directory"
)

# Add flash target
add_custom_target(flash
    DEPENDS ${PROJECT_NAME}
    COMMAND echo "Please manually copy ${CMAKE_SOURCE_DIR}/outputs/${PROJECT_NAME}.uf2 to your Pico"
)