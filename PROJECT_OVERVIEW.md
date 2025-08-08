# PicoLED Protocol Bridge - Project Overview

## What This Project Does

The PicoLED Protocol Bridge enables a single Raspberry Pi Pico to simultaneously control:

1. **WS2812 LED Panels** - Drive serial LED strips/matrices with pixel-perfect control
2. **DMX512 Output** - Send exactly 512 channels of DMX lighting data via RS485
3. **RS485 Serial Communication** - Send custom data frames over RS485 (simplex)

All three protocols work simultaneously on different GPIO pins.

## Key Features

✅ **Professional Architecture** - Clean, object-oriented C++ design  
✅ **Exact DMX512 Compliance** - Always sends exactly 512 channels  
✅ **High Performance** - Uses PIO, DMA, and interrupts for optimal performance  
✅ **Flexible Configuration** - Easily change pins, LED counts, baud rates  
✅ **Ready-to-Use Examples** - Three complete working examples included  
✅ **Git-Friendly** - No large SDK files, users download separately  

## Project Structure

```
PicoLED_ProtocolBridge/
├── 📄 README.md                    # Main documentation
├── 📄 PROJECT_OVERVIEW.md          # This file
├── 📄 build_instructions.md        # Detailed build guide
├── 📄 CMakeLists.txt               # Build configuration
├── 📄 .gitignore                   # Git ignore rules
├── 📁 include/
│   └── PicoLED.h                   # Main class header
├── 📁 src/
│   ├── PicoLED.cpp                 # Main class implementation  
│   ├── config/
│   │   └── picoled_config.h        # Configuration constants
│   └── protocols/
│       ├── dmx512_transmitter.*    # DMX512 implementation
│       ├── ws2812_driver.*         # WS2812 LED driver
│       └── rs485_serial.*          # RS485 communication
└── 📁 examples/
    ├── basic_usage.cpp             # Basic demonstration
    ├── dmx_led_sync.cpp           # LED-DMX synchronization
    └── rs485_test.cpp             # RS485 communication test
```

## Protocol Details

### WS2812 LED Control
- **Connection**: GPIO pin → LED data line
- **Features**: Grid addressing, DMA updates, brightness control
- **Performance**: Up to 60 FPS updates
- **Max LEDs**: 1024 (configurable)

### DMX512 Output  
- **Connection**: GPIO pin → RS485 transceiver → DMX line
- **Compliance**: Exactly 512 channels, DMX512-A standard
- **Timing**: 250,000 baud, proper break/MAB timing
- **Features**: Individual channel control, universe management

### RS485 Serial
- **Connection**: GPIO pins → RS485 transceiver
- **Mode**: Simplex (transmit only)
- **Features**: Variable frame length, automatic direction control
- **Baud Rate**: Configurable (default 115200)

## Default Pin Configuration

| Protocol | GPIO Pin | Function |
|----------|----------|----------|
| WS2812   | 2        | LED Data Output |
| DMX512   | 4        | DMX via RS485 |
| RS485    | 8        | Serial Data |
| RS485    | 9        | Direction Control |

*All pins are configurable in software*

## Usage Examples

### Basic LED Control
```cpp
picoled.setLEDColorXY(x, y, red, green, blue);
picoled.updateLEDPanel();
```

### DMX Output
```cpp
picoled.setDMXChannel(1, 255);    // Channel 1 to full
picoled.transmitDMX();            // Send all 512 channels
```

### RS485 Communication
```cpp
picoled.sendRS485String("Hello World!");
picoled.sendRS485Frame(data, length);
```

### Data Conversion
```cpp
// Convert LED colors to DMX channels
picoled.ledsToDMX(1);  // Start from channel 1
picoled.transmitDMX();

// Convert DMX data to LED colors  
picoled.dmxToLEDs(dmx_data, 1, num_leds);
picoled.updateLEDPanel();
```

## Build Requirements

- **CMake** 3.13+
- **ARM GCC Toolchain** (gcc-arm-none-eabi)
- **Pico SDK** (downloaded separately)
- **Git**

## Quick Start

1. **Install Prerequisites** (see build_instructions.md)
2. **Download Pico SDK** separately:
   ```bash
   git clone https://github.com/raspberrypi/pico-sdk.git ../pico-sdk
   cd ../pico-sdk && git submodule update --init
   ```
3. **Build Project**:
   ```bash
   mkdir build && cd build
   cmake .. && make -j4
   ```
4. **Flash .uf2 files** to your Pico

## Generated Executables

- **basic_usage.uf2** - Demonstrates all three protocols working together
- **dmx_led_sync.uf2** - LED panel synchronized with DMX output
- **rs485_test.uf2** - Comprehensive RS485 communication testing

## Hardware Requirements

- Raspberry Pi Pico (RP2040)
- WS2812 LED strips/panels
- RS485 transceiver modules (e.g., MAX485)
- Appropriate power supplies and level shifters

## Performance Characteristics

- **CPU Usage**: Minimal (interrupt/DMA driven)
- **LED Refresh**: Up to 60 FPS
- **DMX Refresh**: Up to 44 Hz (standard compliant)
- **RS485 Speed**: Up to 115200 baud (configurable)
- **Memory**: ~8KB RAM, ~32KB Flash per protocol

## Professional Features

- ✅ Non-blocking operation using interrupts and DMA
- ✅ Comprehensive error handling and status reporting
- ✅ Thread-safe design for multi-core usage
- ✅ Configurable timing and buffer sizes
- ✅ Built-in diagnostics and debug output
- ✅ Standard-compliant protocol implementation

## Use Cases

- **Stage Lighting Control** - DMX fixtures + LED panels
- **Architectural Lighting** - Building illumination systems  
- **Entertainment Systems** - Interactive displays with lighting
- **Industrial Control** - Equipment status indication
- **Art Installations** - Creative lighting projects
- **Educational Projects** - Learning embedded protocols

## Why This Project?

1. **All-in-One Solution** - Three protocols from one microcontroller
2. **Professional Quality** - Production-ready code architecture
3. **Standard Compliant** - Proper DMX512, WS2812, and RS485 implementation
4. **Well Documented** - Comprehensive examples and documentation
5. **Performance Focused** - Optimized for real-time operation
6. **Developer Friendly** - Clean APIs and extensive configuration options

## License & Support

This project is provided as-is for educational and development purposes. 

For support:
- Read the comprehensive documentation
- Check the working examples
- Review the inline code comments
- Ensure hardware connections are correct

## Contributing

Contributions welcome! Please ensure:
- Code follows existing architecture
- All protocols remain functional  
- Examples are updated appropriately
- DMX512 compliance is maintained (exactly 512 channels)