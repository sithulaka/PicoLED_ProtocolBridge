cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
# Users should set PICO_SDK_PATH environment variable or place pico-sdk in parent directory
if(NOT DEFINED PICO_SDK_PATH)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../pico-sdk/pico_sdk_init.cmake")
        set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../pico-sdk")
        message("Using Pico SDK from: ${PICO_SDK_PATH}")
    elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/pico-sdk/pico_sdk_init.cmake")
        set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
        message("Using Pico SDK from: ${PICO_SDK_PATH}")
    else()
        message(FATAL_ERROR "Pico SDK not found! Please:\n"
                            "1. Set PICO_SDK_PATH environment variable, or\n"
                            "2. Place pico-sdk folder in parent directory, or\n"
                            "3. Place pico-sdk folder in project directory\n"
                            "See README.md for installation instructions.")
    endif()
endif()

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(picoled_protocol_bridge C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Explicitly enable C++ language
enable_language(CXX)

# Initialize the SDK
pico_sdk_init()

# Set include directories
include_directories(include)
include_directories(src/config)
include_directories(src/protocols)

# Core protocol sources
set(PROTOCOL_SOURCES
    src/protocols/dmx512_transmitter.cpp
    src/protocols/ws2812_driver.cpp
    src/protocols/rs485_serial.cpp
)

# Main PicoLED class
set(PICOLED_SOURCES
    src/PicoLED.cpp
    ${PROTOCOL_SOURCES}
)

# Example executables
add_executable(basic_usage
    examples/basic_usage.cpp
    ${PICOLED_SOURCES}
)

add_executable(dmx_led_sync
    examples/dmx_led_sync.cpp
    ${PICOLED_SOURCES}
)

add_executable(rs485_test
    examples/rs485_test.cpp
    ${PICOLED_SOURCES}
)

# Link libraries for all executables
set(COMMON_LIBRARIES
    pico_stdlib
    hardware_pio
    hardware_dma
    hardware_uart
    hardware_gpio
    hardware_irq
    hardware_clocks
    pico_multicore
)

target_link_libraries(basic_usage ${COMMON_LIBRARIES})
target_link_libraries(dmx_led_sync ${COMMON_LIBRARIES})
target_link_libraries(rs485_test ${COMMON_LIBRARIES})

# Enable USB output for debugging
pico_enable_stdio_usb(basic_usage 1)
pico_enable_stdio_uart(basic_usage 0)

pico_enable_stdio_usb(dmx_led_sync 1)
pico_enable_stdio_uart(dmx_led_sync 0)

pico_enable_stdio_usb(rs485_test 1)
pico_enable_stdio_uart(rs485_test 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(basic_usage)
pico_add_extra_outputs(dmx_led_sync)
pico_add_extra_outputs(rs485_test)

# Print build information
message(STATUS "Building PicoLED Protocol Bridge")
message(STATUS "  - WS2812 LED Panel Control")
message(STATUS "  - DMX512 Output (exactly 512 channels)")
message(STATUS "  - RS485 Serial Communication (simplex)")
message(STATUS "Executables will be generated:")
message(STATUS "  - basic_usage.uf2")
message(STATUS "  - dmx_led_sync.uf2")
message(STATUS "  - rs485_test.uf2")