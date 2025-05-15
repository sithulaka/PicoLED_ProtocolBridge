#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored messages
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to check command status
check_status() {
    if [ $? -ne 0 ]; then
        print_message "$RED" "$1"
        exit 1
    fi
}

print_message "$YELLOW" "Starting LED Project Build Process..."

# Check if Pico-DMX is installed
if [ ! -d "Pico-DMX" ]; then
    print_message "$RED" "Pico-DMX not found! Please install the Pico-DMX library first."
    print_message "$YELLOW" "1. Download the Pico-DMX library"
    print_message "$YELLOW" "2. Extract it to the Pico-DMX directory"
    exit 1
fi

# Check if pico-sdk is initialized (check for pico_sdk_init.cmake)
if [ ! -f "pico-sdk/pico_sdk_init.cmake" ]; then
    print_message "$RED" "Error: pico-sdk not found! Please ensure it's properly initialized."
    exit 1
fi

# Create and navigate to build directory
if [ ! -d "build" ]; then
    print_message "$GREEN" "Creating build directory..."
    mkdir build
fi
cd build

# Run cmake
print_message "$YELLOW" "Running CMake..."
cmake .. 
check_status "CMake configuration failed!"

# Run make with parallel jobs
print_message "$YELLOW" "Building project..."
make -j$(nproc)
check_status "Build failed!"

# Create output directory if it doesn't exist
if [ ! -d "../outputs" ]; then
    print_message "$GREEN" "Creating output directory..."
    mkdir -p "../outputs"
fi

# Copy the generated files to the output directory
print_message "$YELLOW" "Copying output files..."
cp *.uf2 ../outputs/ 2>/dev/null || true

# Check if UF2 file exists
if [ -f "../outputs/PicoLED_ProtocolBridge.uf2" ] || [ -f "PicoLED_ProtocolBridge.uf2" ]; then
    print_message "$GREEN" "Build successful!"
    print_message "$YELLOW" "Output files location: outputs/"
    
    # List the generated files
    print_message "$GREEN" "Generated files:"
    ls -lh ../outputs/
    
    print_message "$YELLOW" "To flash the Pico:"
    print_message "$GREEN" "1. Hold the BOOTSEL button"
    print_message "$GREEN" "2. Connect the Pico to USB"
    print_message "$GREEN" "3. Copy PicoLED_ProtocolBridge.uf2 to the RPI-RP2 drive"
else
    print_message "$RED" "Build failed to generate UF2 file!"
    exit 1
fi