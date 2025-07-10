# **🎮 PicoLED Protocol Bridge**

## ⚠️ Important Note
This repository implements a LED Protocol Bridge for the Raspberry Pi Pico. Please ensure that you:
- Install the required dependencies
- Set up the Pico SDK correctly
- Configure the hardware properly

## ✅ Implemented Features
The following features are included in this codebase:

1. **Core Functionality**
   - WS2812B LED strip control
   - DMX protocol support
   - 8x8 matrix support (configurable)

2. **Code Features**
   - Centralized configuration via `config.h`
   - Memory-safe implementation
   - Multi-core operation for responsiveness
   - Grid layout support for LED matrices
   - XY addressing for matrix layouts

3. **Build System**
   - CMake-based build system
   - Windows build script (build.bat)
   - Linux build script (build.sh)

## 🔹 Overview
This project implements a **LED Protocol Bridge** using the **Raspberry Pi Pico** microcontroller. It enables control of WS2812B LED strips through various communication protocols, making it perfect for custom lighting solutions and IoT projects.

## ⚙️ Prerequisites

Before building this project, ensure you have the following installed:

### **Required Dependencies**

1. **Pico SDK**
   - The Raspberry Pi Pico C/C++ SDK provides the core functionality
   - Can be downloaded from the official Raspberry Pi website
   - Must be initialized with submodules

2. **Pico-DMX Library**
   - DMX protocol implementation for the Raspberry Pi Pico
   - Required for receiving and processing DMX data
   - Must be installed in the `Pico-DMX` directory

3. **Build Tools**
   - CMake (3.13 or newer)
   - ARM GCC Toolchain
   - Python 3

### **Linux/Ubuntu Setup**
```sh
# Install required packages
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential git python3

# Clone and setup Pico SDK
cd ~/
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=$HOME/pico-sdk

# Download and set up Pico-DMX
# Place in the Pico-DMX directory in your project
git https://github.com/jostlowe/Pico-DMX.git

```

### **Windows Setup**
1. Install [ARM GCC Compiler](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
2. Install [CMake](https://cmake.org/download/)
3. Install [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
4. Install [Python 3](https://www.python.org/downloads/)
5. Install [Git](https://git-scm.com/downloads)

Then set up the Pico SDK:
```batch
mkdir %USERPROFILE%\pico
cd %USERPROFILE%\pico
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
setx PICO_SDK_PATH "%USERPROFILE%\pico\pico-sdk"
```

Finally, download the Pico-DMX library and place it in the `Pico-DMX` directory in your project.

## 📥 Clone and Build

### **Linux**
```sh
# Clone this repository
git clone https://github.com/sithulaka/PicoLED_ProtocolBridge.git
cd PicoLED_ProtocolBridge

# Create build directory and build
mkdir build
cd build
cmake ..
make -j4
```

### **Windows**
```batch
# Clone this repository
git clone https://github.com/sithulaka/PicoLED_ProtocolBridge.git
cd PicoLED_ProtocolBridge

# Create build directory and build
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```

## 📂 Project Structure
```
PicoLED_ProtocolBridge/
│── src/             # Source files
│   ├── main.cpp     # Main program
│   └── PicoLED.cpp  # LED control implementation
│── include/         # Header files
│   ├── PicoLED.h    # LED control declarations
│   └── config.h     # Configuration parameters
│── extras/          # Additional resources
│   └── ws2812.pio   # PIO program for WS2812
│── build.sh         # Linux build script
│── build.bat        # Windows build script 
│── CMakeLists.txt   # CMake build configuration
└── README.md        # This file
```

## 🔌 Hardware Setup
1. Connect the WS2812B LED strip's data pin to GPIO16 (pin 21)
2. Connect DMX input to GPIO1 (pin 2)
3. Connect power and ground according to the diagram below:

```
                  +-----------------+
                  |                 |
                  |     PICO        |
                  |                 |
+-----------+     |                 |     +------------------+
|           |     |                 |     |                  |
| DMX INPUT |-----| GPIO1 (pin 2)   |     |                 |
|    IN     |     |                 |     |                 |
+-----------+     |                 |     |                 |
                  |                 |     |    WS2812B      |
                  | GPIO16 (pin 21) |-----| DATA           |
                  |                 |     |                 |
                  | VSYS (pin 39)   |-----| VCC (+5V)       |
                  |                 |     |                 |
                  | GND (pin 38)    |-----| GND             |
                  |                 |     |                 |
                  +-----------------+     +------------------+
```

**Important Power Considerations**:
- WS2812B LEDs can draw significant current. For a full 64 LED matrix at full brightness:
  - Each LED can draw up to 60mA at full white
  - 64 LEDs × 60mA = 3.84A potentially required
- Do not power the LEDs from the Pico's pins directly
- Use a separate 5V power supply rated for your LED count
- Connect the power supply ground to both the Pico GND and LED strip GND

## 🚀 Flashing Instructions

### **Linux**
1. Hold the BOOTSEL button on the Pico
2. Connect the Pico to your computer via USB
3. Release BOOTSEL
4. Copy the generated .uf2 file:
```sh
cp build/PicoLED_ProtocolBridge.uf2 /media/$USER/RPI-RP2/
```

### **Windows**
1. Hold the BOOTSEL button on the Pico
2. Connect the Pico to your computer via USB
3. Release BOOTSEL
4. A new drive "RPI-RP2" will appear
5. Copy the generated `PicoLED_ProtocolBridge.uf2` from the `build` folder to the RPI-RP2 drive

## 🔧 Troubleshooting
- If CMake can't find the Pico SDK, ensure PICO_SDK_PATH is set correctly
- For Windows users, restart your terminal after setting environment variables
- Make sure all submodules are initialized with `git submodule update --init`

## 🤝 Contributing
Contributions are welcome! Feel free to submit issues and pull requests to [the GitHub repository](https://github.com/sithulaka/PicoLED_ProtocolBridge).

## 📝 License
This project is licensed under the MIT License - see the LICENSE file for details.

## 🌐 Connect with me
- GitHub: [sithulaka](https://github.com/sithulaka)