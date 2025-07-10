# PicoLED Protocol Bridge

A dual-device system for controlling WS2812 LEDs via DMX512 protocol using Raspberry Pi Pico RP2040 microcontrollers.

## 🎯 Project Overview

This project implements a DMX512 to WS2812 LED protocol bridge using two Raspberry Pi Pico devices:

1. **DMX Sender (`dmx-sender.cpp`)**: Creates LED animations on a local LED matrix, converts the LED data to DMX universe format, and transmits it via DMX512.

2. **DMX Receiver (`dmx-reciver.cpp`)**: Receives DMX512 data on one CPU core, processes it on another core, and outputs the data to WS2812 serial LEDs in real-time.

## 🚀 Features

### DMX Sender
- ✨ **LED Animation Engine**: Creates complex text/shape animations on 8x8 LED matrix
- 🔄 **Real-time DMX Conversion**: Converts PicoLED array data to DMX512 universe format
- 📡 **DMX512 Transmission**: Sends DMX data over RS-485 protocol
- 📊 **Comprehensive Logging**: Detailed status and performance monitoring

### DMX Receiver  
- 🎯 **Dual-Core Architecture**: 
  - Core 0: High-speed DMX packet reception
  - Core 1: LED data processing and output
- ⚡ **Real-time Processing**: Sub-20ms latency from DMX to LED update
- 🔒 **Thread-Safe Communication**: Mutex-protected inter-core data sharing
- 🌈 **Full Color Support**: RGB color mapping with 8-bit resolution per channel

## 📋 Hardware Requirements

### For Each Pico Device:
- **Raspberry Pi Pico RP2040** (1x per device)
- **MAX485 or SN75176 IC** (for DMX512 communication)
- **WS2812/WS2812B LED Strip or Matrix** (for receiver only)
- **120Ω termination resistor** (for DMX line)
- **3.3V to 5V level shifter** (for LED data line)

### Pin Configuration (configurable in `config.h`):
```cpp
#define DMX_IN_PIN 1        // GPIO pin for DMX input/output
#define WS2812_PIN 16       // GPIO pin for WS2812 LED data
#define NUM_PIXELS 64       // Total number of LEDs (8x8 matrix)
#define NUM_CHANNELS 255    // DMX channels to use
```

## 🛠️ Software Dependencies

- **Pico SDK** (latest version)
- **CMake** (3.13 or higher)  
- **Ninja** or **Make** build system
- **ARM GCC Toolchain**
- **PicoLED Library** (included)
- **Pico-DMX Library** (included as submodule)

## 📦 Installation & Setup

### 1. Clone and Setup
```powershell
git clone <repository-url>
cd PicoLED_ProtocolBridge
```

### 2. Install Pico SDK
```powershell
# Download and setup Pico SDK
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
cd ..

# Set environment variable
$env:PICO_SDK_PATH = "C:\path\to\pico-sdk"
```

### 3. Verify Pico-DMX Library
Ensure the `Pico-DMX` directory exists and contains the DMX library files.

## 🔨 Building the Project

### Using PowerShell Script (Recommended)
```powershell
.\build.ps1
```

### Manual Build
```powershell
mkdir build
cd build
cmake -G "Ninja" ..
cmake --build . --target dmx_sender
cmake --build . --target dmx_receiver
```

## 📱 Flashing to Pico

1. **Hold BOOTSEL button** while connecting Pico to USB
2. **Copy appropriate .uf2 file** to the Pico drive:
   - `dmx_sender.uf2` → DMX Sender Pico
   - `dmx_receiver.uf2` → DMX Receiver Pico

## 🔧 Configuration

### LED Matrix Setup (`config.h`)
```cpp
#define NUM_PIXELS 64       // Total LEDs (8x8 = 64)
#define GRID_WIDTH 8        // Matrix width
#define GRID_HEIGHT 8       // Matrix height
```

### DMX Configuration
```cpp
#define START_CHANNEL 1     // First DMX channel
#define NUM_CHANNELS 255    // Channels to read/send
```

### Hardware Pins
```cpp
#define DMX_IN_PIN 1        // DMX data pin
#define WS2812_PIN 16       // LED data pin
```

## 🎨 LED Animation Pattern

The DMX sender creates an animated text pattern:

1. **White Flash**: All LEDs illuminate white for 5 seconds
2. **Character Animation**: Draws text/shapes pixel by pixel with 500ms delays
3. **Color Cycling**: Characters change through different colors
4. **Loop**: Animation repeats continuously

Each pixel uses RGB values `(0, 0, 100)` for blue color in the animation.

## 🔌 Wiring Diagrams

### DMX Sender Wiring
```
Pico GPIO 1 ── MAX485 DI
Pico GPIO 16 ── WS2812 Data In (via level shifter)
Pico 3.3V ── MAX485 VCC
Pico GND ── MAX485 GND
MAX485 A ── DMX+ 
MAX485 B ── DMX-
```

### DMX Receiver Wiring  
```
DMX+ ── MAX485 A
DMX- ── MAX485 B
MAX485 RO ── Pico GPIO 1
MAX485 VCC ── Pico 3.3V
MAX485 GND ── Pico GND
Pico GPIO 16 ── WS2812 Data In (via level shifter)
```

## 📊 Performance Metrics

### DMX Sender
- **Animation Frame Rate**: ~2 FPS (configurable via delays)
- **DMX Update Rate**: After each LED change (~500ms intervals)
- **DMX Universe Size**: 513 bytes (1 start code + 512 data)

### DMX Receiver
- **DMX Reception Rate**: Up to 44 Hz (standard DMX refresh rate)
- **LED Update Latency**: <20ms from DMX packet to LED output
- **Core 0 (DMX)**: Dedicated packet reception and validation
- **Core 1 (LED)**: Color conversion and LED output (50 FPS max)

## 🐛 Troubleshooting

### Common Issues

**No DMX Signal**
- Check MAX485 wiring and power
- Verify DMX termination (120Ω resistor)
- Check DMX cable polarity

**LEDs Not Responding**
- Verify WS2812 power supply (5V, adequate current)
- Check data line level shifting (3.3V → 5V)
- Confirm LED strip connection and orientation

**Build Errors**
- Ensure PICO_SDK_PATH is set correctly
- Check CMake and toolchain installation
- Verify all dependencies are installed

### Debug Output

Both devices provide detailed logging via USB serial:

```
[DMX-SENDER] Starting LED pattern animation...
[DMX-SENDER] Converting LED array to DMX universe...
[CORE0-DMX] Packet #1234 received, 255 channels
[CORE1-LED] LED update #567 completed
```

## 📄 License

This project is licensed under the MIT License. See LICENSE file for details.

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📧 Support

For issues and questions:
- Create an issue on GitHub
- Check the troubleshooting section
- Review the debug output logs

---

**Made with ❤️ for the LED and DMX community**
