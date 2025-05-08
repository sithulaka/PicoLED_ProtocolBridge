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
if [ ! -d "../led_project/output" ]; then
    print_message "$GREEN" "Creating output directory..."
    mkdir -p "../led_project/output"
fi

# Copy the generated files to the output directory
print_message "$YELLOW" "Copying output files..."
cp *.uf2 ../led_project/output/ 2>/dev/null || true

# Check if UF2 file exists
if [ -f "../led_project/output/led_project.uf2" ] || [ -f "led_project.uf2" ]; then
    print_message "$GREEN" "Build successful!"
    print_message "$YELLOW" "Output files location: led_project/output/"
    
    # List the generated files
    print_message "$GREEN" "Generated files:"
    ls -lh ../led_project/output/
    
    print_message "$YELLOW" "To flash the Pico:"
    print_message "$GREEN" "1. Hold the BOOTSEL button"
    print_message "$GREEN" "2. Connect the Pico to USB"
    print_message "$GREEN" "3. Copy led_project.uf2 to the RPI-RP2 drive"
else
    print_message "$RED" "Build failed to generate UF2 file!"
    exit 1
fi