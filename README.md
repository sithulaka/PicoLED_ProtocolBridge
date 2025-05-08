# **🎮 PicoLED Protocol Bridge**

## 🔹 Overview
This project implements a **LED Protocol Bridge** using the **Raspberry Pi Pico** microcontroller. It enables control of WS2812B LED strips through various communication protocols, making it perfect for custom lighting solutions and IoT projects.

## ⚙️ Prerequisites

Before building this project, ensure you have the following installed:

### **Linux/Ubuntu**
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
```

### **Windows**
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

## 📥 Clone and Build

### **Linux**
```sh
# Clone this repository
git clone https://github.com/yourusername/PicoLED_ProtocolBridge.git
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
git clone https://github.com/yourusername/PicoLED_ProtocolBridge.git
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
│   ├── led.cpp      # LED control implementation
│   └── ws2812.pio   # PIO program for WS2812
│── include/         # Header files
│   └── led.h        # LED control declarations
│── CMakeLists.txt   # CMake build configuration
└── README.md        # This file
```

## 🔌 Hardware Setup
1. Connect the WS2812B LED strip's data pin to GPIO pin [Your GPIO Pin]
2. Connect VCC (5V) and GND appropriately
3. Ensure proper power supply for the LED strip

## 🚀 Flashing Instructions

### **Linux**
1. Hold the BOOTSEL button on the Pico
2. Connect the Pico to your computer via USB
3. Release BOOTSEL
4. Copy the generated .uf2 file:
```sh
cp build/led_project.uf2 /media/$USER/RPI-RP2/
```

### **Windows**
1. Hold the BOOTSEL button on the Pico
2. Connect the Pico to your computer via USB
3. Release BOOTSEL
4. A new drive "RPI-RP2" will appear
5. Copy the generated `led_project.uf2` from the `build` folder to the RPI-RP2 drive

## 🔧 Troubleshooting
- If CMake can't find the Pico SDK, ensure PICO_SDK_PATH is set correctly
- For Windows users, restart your terminal after setting environment variables
- Make sure all submodules are initialized with `git submodule update --init`

## 🤝 Contributing
Contributions are welcome! Feel free to submit issues and pull requests.

## 📝 License
This project is licensed under the MIT License - see the LICENSE file for details.

## 🌐 Connect with me
<p align="left">
<a href="https://linkedin.com/in/yourusername" target="blank"><img align="center" src="https://raw.githubusercontent.com/rahuldkjain/github-profile-readme-generator/master/src/images/icons/Social/linked-in-alt.svg" alt="yourusername" height="30" width="40" /></a>
<a href="https://github.com/yourusername" target="blank"><img align="center" src="https://raw.githubusercontent.com/rahuldkjain/github-profile-readme-generator/master/src/images/icons/Social/github.svg" alt="yourusername" height="30" width="40" /></a>
</p>