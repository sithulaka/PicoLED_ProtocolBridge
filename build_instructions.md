# Build Instructions

## Quick Setup Script

For convenience, here's a script that handles the entire setup:

### Linux/macOS Setup Script

```bash
#!/bin/bash
# setup.sh - PicoLED Protocol Bridge setup script

set -e

echo "=== PicoLED Protocol Bridge Setup ==="

# Check prerequisites
echo "Checking prerequisites..."

if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found. Please install CMake 3.13 or later."
    exit 1
fi

if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "❌ ARM GCC toolchain not found. Please install gcc-arm-none-eabi."
    exit 1
fi

if ! command -v git &> /dev/null; then
    echo "❌ Git not found. Please install Git."
    exit 1
fi

echo "✅ All prerequisites found"

# Download Pico SDK if not present
if [ ! -d "../pico-sdk" ] && [ ! -d "pico-sdk" ] && [ -z "$PICO_SDK_PATH" ]; then
    echo "📥 Downloading Pico SDK..."
    cd ..
    git clone https://github.com/raspberrypi/pico-sdk.git
    cd pico-sdk
    git submodule update --init
    echo "✅ Pico SDK downloaded and initialized"
    cd ../PicoLED_ProtocolBridge
else
    echo "✅ Pico SDK found"
fi

# Build project
echo "🔨 Building project..."
mkdir -p build
cd build
cmake ..
make -j$(nproc)

echo ""
echo "🎉 Build complete! Generated files:"
echo "  - basic_usage.uf2"
echo "  - dmx_led_sync.uf2" 
echo "  - rs485_test.uf2"
echo ""
echo "To flash: Hold BOOTSEL button on Pico, connect USB, copy .uf2 file to RPI-RP2 drive"
```

Save this as `setup.sh`, make it executable (`chmod +x setup.sh`), and run it (`./setup.sh`).

## Manual Step-by-Step Instructions

### 1. Install Prerequisites

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential git
```

#### macOS (with Homebrew)
```bash
brew install cmake
brew install --cask gcc-arm-embedded
```

#### Windows
1. Install [CMake](https://cmake.org/download/)
2. Install [ARM GCC Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
3. Install [Git](https://git-scm.com/download/win)
4. Add all tools to your PATH

### 2. Download and Setup Pico SDK

```bash
# Navigate to parent directory
cd ..

# Clone Pico SDK
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk

# Initialize submodules (this may take several minutes)
git submodule update --init

# Return to project directory
cd ../PicoLED_ProtocolBridge
```

**Alternative**: Set environment variable
```bash
export PICO_SDK_PATH=/path/to/your/pico-sdk
```

### 3. Build Project

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build (adjust -j4 to your CPU core count)
make -j4
```

### 4. Flash to Pico

1. Hold the **BOOTSEL** button on your Raspberry Pi Pico
2. Connect the Pico to your computer via USB while holding BOOTSEL
3. The Pico will mount as a USB drive named **RPI-RP2**
4. Copy the desired `.uf2` file to the RPI-RP2 drive
5. The Pico will automatically reboot and run your program

## Build Verification

After a successful build, you should see:

```
-- Configuring done
-- Generating done
-- Build files have been written to: /path/to/build
[100%] Built target basic_usage
[100%] Built target dmx_led_sync 
[100%] Built target rs485_test
```

And the following files in your `build` directory:
- `basic_usage.elf` and `basic_usage.uf2`
- `dmx_led_sync.elf` and `dmx_led_sync.uf2`
- `rs485_test.elf` and `rs485_test.uf2`

## Troubleshooting Build Issues

### "Pico SDK not found" Error
```
CMake Error: Pico SDK not found! Please:
1. Set PICO_SDK_PATH environment variable, or
2. Place pico-sdk folder in parent directory, or  
3. Place pico-sdk folder in project directory
```

**Solution**: Follow step 2 above to download the Pico SDK, or set the PICO_SDK_PATH environment variable.

### "arm-none-eabi-gcc not found" Error
```
Could not find toolchain file: /pico-sdk/cmake/preload/toolchains/pico_arm_gcc.cmake
```

**Solution**: Install the ARM GCC toolchain as shown in step 1.

### Submodule Errors
```
fatal: No url found for submodule path 'lib/tinyusb'
```

**Solution**: Make sure to run `git submodule update --init` in the pico-sdk directory.

### Permission Errors (Linux/macOS)
```
Permission denied
```

**Solution**: Don't run cmake or make with sudo. If you need to install packages, use sudo only for the package manager (apt, brew, etc.).

## Advanced Build Options

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4
```

### Release Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

### Custom SDK Path
```bash
cmake -DPICO_SDK_PATH=/custom/path/to/pico-sdk ..
make -j4
```

### Verbose Build Output
```bash
make VERBOSE=1
```

## Build System Details

- **CMake Version**: 3.13 or later required
- **C Standard**: C11
- **C++ Standard**: C++17
- **Target**: ARM Cortex-M0+ (RP2040)
- **Build Tools**: ARM GCC Embedded Toolchain

The build system automatically:
- Detects the Pico SDK location
- Configures all necessary libraries
- Generates both ELF and UF2 files
- Enables USB serial output for debugging