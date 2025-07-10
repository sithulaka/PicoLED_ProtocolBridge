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

# Common source files
set(COMMON_SOURCES
    src/PicoLED.cpp
)

# Common header files
file(GLOB_RECURSE PROJECT_HEADERS
    "include/*.h"
    "include/*.hpp"
)

# Create DMX Sender executable
add_executable(dmx_sender
    src/dmx-sender.cpp
    ${COMMON_SOURCES}
    ${PROJECT_HEADERS}
)

# Create DMX Receiver executable  
add_executable(dmx_receiver
    src/dmx-reciver.cpp
    ${COMMON_SOURCES}
    ${PROJECT_HEADERS}
)

# Generate PIO header files for both targets
pico_generate_pio_header(dmx_sender ${CMAKE_CURRENT_LIST_DIR}/extras/ws2812.pio)
pico_generate_pio_header(dmx_sender ${CMAKE_CURRENT_LIST_DIR}/Pico-DMX/extras/DmxOutput.pio)

pico_generate_pio_header(dmx_receiver ${CMAKE_CURRENT_LIST_DIR}/extras/ws2812.pio)
pico_generate_pio_header(dmx_receiver ${CMAKE_CURRENT_LIST_DIR}/Pico-DMX/extras/DmxInput.pio)

# Include directories for both targets
target_include_directories(dmx_sender PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Pico-DMX/src
)

target_include_directories(dmx_receiver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Pico-DMX/src
)

# Link libraries for both targets
target_link_libraries(dmx_sender PRIVATE
    pico_stdlib
    hardware_pio
    pico_multicore
    hardware_dma
    hardware_irq
    hardware_uart
    picodmx
)

target_link_libraries(dmx_receiver PRIVATE
    pico_stdlib
    hardware_pio
    pico_multicore
    hardware_dma
    hardware_irq
    hardware_uart
    picodmx
)

# USB and UART configuration for both targets
pico_enable_stdio_usb(dmx_sender 1)
pico_enable_stdio_uart(dmx_sender 0)

pico_enable_stdio_usb(dmx_receiver 1)
pico_enable_stdio_uart(dmx_receiver 0)

# Generate additional output files for both targets
pico_add_extra_outputs(dmx_sender)
pico_add_extra_outputs(dmx_receiver)

# Create an output directory in the project root
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/outputs)

# Add custom commands to copy output files for both targets
add_custom_command(TARGET dmx_sender POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying DMX Sender build artifacts to outputs directory..."
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:dmx_sender>/dmx_sender.elf
        ${CMAKE_SOURCE_DIR}/outputs/

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/dmx_sender.uf2
        ${CMAKE_SOURCE_DIR}/outputs/
        || echo "Warning: DMX Sender UF2 file not found"
        
    COMMENT "Copying DMX Sender build artifacts to outputs directory"
)

add_custom_command(TARGET dmx_receiver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying DMX Receiver build artifacts to outputs directory..."
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:dmx_receiver>/dmx_receiver.elf
        ${CMAKE_SOURCE_DIR}/outputs/

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/dmx_receiver.uf2
        ${CMAKE_SOURCE_DIR}/outputs/
        || echo "Warning: DMX Receiver UF2 file not found"
        
    COMMENT "Copying DMX Receiver build artifacts to outputs directory"
)

# Add flash targets for both
add_custom_target(flash_sender
    DEPENDS dmx_sender
    COMMAND echo "Please manually copy ${CMAKE_SOURCE_DIR}/outputs/dmx_sender.uf2 to your Pico"
)

add_custom_target(flash_receiver
    DEPENDS dmx_receiver
    COMMAND echo "Please manually copy ${CMAKE_SOURCE_DIR}/outputs/dmx_receiver.uf2 to your Pico"
)