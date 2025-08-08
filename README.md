# PicoLED Protocol Bridge

A professional Raspberry Pi Pico-based protocol bridge that simultaneously supports:

- **WS2812 LED Panel Control** - Serial LED panel data output via PIO
- **DMX512 Output** - Exactly 512 channels via RS485 
- **RS485 Serial Communication** - Simplex communication with variable frame lengths

## Features

- **Multi-Protocol Support**: Control WS2812 LEDs, transmit DMX512, and send RS485 data simultaneously
- **Professional Architecture**: Well-organized, object-oriented C++ design
- **Exact DMX512 Compliance**: Precisely 512 channels, no more, no less
- **High Performance**: Uses PIO for WS2812, DMA where available, interrupt-driven operation
- **Flexible Configuration**: Configurable pins, LED grid sizes, and communication parameters
- **Comprehensive Examples**: Ready-to-use examples for various use cases

## Hardware Requirements

- Raspberry Pi Pico (RP2040)
- WS2812 LED strips/panels
- RS485 transceiver modules (e.g., MAX485)
- Appropriate level shifters if needed

## Pin Configuration

Default pin assignments (configurable):

| Protocol | Pin | Function |
|----------|-----|----------|
| WS2812 | 2 | LED Data Output |
| DMX512 | 4 | DMX Data to RS485 |
| RS485 | 8 | Serial Data |
| RS485 | 9 | Direction Control (optional) |

## Quick Start

### 1. Prerequisites

You need the following tools installed:
- **CMake** (version 3.13 or later)
- **GCC ARM Embedded Toolchain** 
- **Git**

On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential git
```

On macOS:
```bash
brew install cmake
brew install --cask gcc-arm-embedded
```

### 2. Download Pico SDK

The Pico SDK is required but not included in this repository. Download it separately:

```bash
# Option 1: Place SDK in parent directory (recommended)
cd ..
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
cd ../PicoLED_ProtocolBridge

# Option 2: Set environment variable
export PICO_SDK_PATH=/path/to/your/pico-sdk
```

### 3. Build the Project

```bash
mkdir build
cd build
cmake ..
make -j4
```

### 4. Flash to Pico

After successful build, you'll find `.uf2` files in the build directory:

- `basic_usage.uf2` - Basic demonstration of all protocols
- `dmx_led_sync.uf2` - LED-DMX synchronization demo  
- `rs485_test.uf2` - RS485 communication test

To flash:
1. Hold the BOOTSEL button on your Pico and connect via USB
2. Copy the desired `.uf2` file to the mounted RPI-RP2 drive
3. The Pico will automatically reboot and run your program

### 5. Basic Usage Example

```cpp
#include "PicoLED.h"

// Configure pins
PicoLED::PinConfig pins = {
    .led_panel_pin = 2,
    .dmx512_pin = 4,
    .rs485_data_pin = 8,
    .rs485_enable_pin = 9
};

// Configure LED panel
PicoLED::LEDConfig led_config = {
    .num_pixels = 256,
    .grid_width = 16,
    .grid_height = 16,
    .pio_instance = pio0,
    .pio_sm = 0
};

// Create and initialize
PicoLED picoled(pins, led_config);
picoled.begin();

// Set LED colors
picoled.setLEDColorXY(0, 0, 255, 0, 0);  // Red pixel at (0,0)
picoled.updateLEDPanel();

// Set DMX channels
picoled.setDMXChannel(1, 255);   // Channel 1 = 255
picoled.transmitDMX();           // Send exactly 512 channels

// Send RS485 data
picoled.sendRS485String("Hello World!");
```

## Architecture

```
PicoLED (Main Class)
├── WS2812Driver (LED Panel Control)
│   ├── PIO-based high-speed output
│   ├── DMA support for smooth updates
│   └── Grid/matrix helper functions
├── DMX512Transmitter (Exactly 512 channels)
│   ├── UART-based with precise timing
│   ├── Interrupt-driven transmission
│   └── Full DMX512-A compliance
└── RS485Serial (Simplex Communication)
    ├── Variable frame length support
    ├── Automatic direction control
    └── DMA-accelerated transfers
```

## Project Structure

```
PicoLED_ProtocolBridge/
├── include/
│   └── PicoLED.h                    # Main class header
├── src/
│   ├── PicoLED.cpp                  # Main class implementation
│   ├── config/
│   │   └── picoled_config.h         # Configuration constants
│   └── protocols/
│       ├── dmx512_transmitter.h/.cpp # DMX512 implementation
│       ├── ws2812_driver.h/.cpp      # WS2812 LED driver
│       └── rs485_serial.h/.cpp       # RS485 serial driver
├── examples/
│   ├── basic_usage.cpp              # Basic demonstration
│   ├── dmx_led_sync.cpp            # LED-DMX synchronization
│   └── rs485_test.cpp              # RS485 communication test
├── CMakeLists.txt                   # Build configuration
└── README.md                        # This file
```

## Protocol Details

### WS2812 LED Panel
- **Format**: GRB (native WS2812 format)
- **Timing**: 800 kHz data rate with precise timing
- **Features**: Grid addressing, DMA updates, brightness control
- **Max LEDs**: 1024 (configurable)

### DMX512 Output
- **Channels**: Exactly 512 channels (as per DMX512-A standard)
- **Baud Rate**: 250,000 baud (standard DMX)
- **Format**: 8 data bits, 2 stop bits, no parity
- **Break/MAB**: Compliant timing (100μs break, 12μs mark-after-break)
- **Output**: Via RS485 transceiver

### RS485 Serial Communication
- **Mode**: Simplex (transmit only)
- **Frame Length**: Variable (up to 1024 bytes per frame)
- **Baud Rate**: Configurable (default 115200)
- **Features**: Automatic direction control, custom frame formats

## Configuration Options

Key configuration parameters in `picoled_config.h`:

```cpp
#define DMX_UNIVERSE_SIZE           512     // Exactly 512 channels
#define DEFAULT_LED_COUNT           256     // Default LED count
#define DEFAULT_GRID_WIDTH          16      // Default grid width
#define RS485_DEFAULT_BAUD          115200  // RS485 baud rate
#define UPDATE_INTERVAL_MS          16      // ~60 FPS update rate
```

## API Reference

### Main PicoLED Class

#### Initialization
- `PicoLED(pins, led_config)` - Constructor
- `bool begin()` - Initialize all protocols
- `void end()` - Shutdown all protocols

#### LED Panel Control
- `setLEDColor(index, r, g, b)` - Set LED by index
- `setLEDColorXY(x, y, r, g, b)` - Set LED by grid position
- `setAllLEDs(r, g, b)` - Set all LEDs to same color
- `updateLEDPanel()` - Push changes to LED panel

#### DMX512 Output
- `setDMXChannel(channel, value)` - Set single channel (1-512)
- `setDMXUniverse(data)` - Set entire universe (512 channels)
- `transmitDMX()` - Transmit exactly 512 channels
- `isDMXBusy()` - Check transmission status

#### RS485 Communication
- `sendRS485Frame(data, length)` - Send binary data
- `sendRS485String(str)` - Send string data
- `isRS485Busy()` - Check transmission status
- `setRS485BaudRate(baud)` - Change baud rate

#### Data Conversion
- `dmxToLEDs(dmx_data, start_channel, num_leds)` - Convert DMX to LED colors
- `ledsToDMX(start_channel)` - Convert LED colors to DMX channels

## Examples

### Basic RGB LED Control
```cpp
// Set rainbow pattern
for (uint x = 0; x < 16; x++) {
    for (uint y = 0; y < 16; y++) {
        uint8_t r = (x * 16);
        uint8_t g = (y * 16);
        uint8_t b = ((x + y) * 8);
        picoled.setLEDColorXY(x, y, r, g, b);
    }
}
picoled.updateLEDPanel();
```

### DMX-LED Synchronization
```cpp
// Convert LED data to DMX and transmit
picoled.ledsToDMX(1);  // Start from DMX channel 1
picoled.transmitDMX(); // Send exactly 512 channels
```

### RS485 Data Logging
```cpp
// Send sensor data
char data[64];
snprintf(data, sizeof(data), "TEMP:%.1f,HUMIDITY:%.1f\n", temp, humidity);
picoled.sendRS485String(data);
```

## Performance

- **LED Update Rate**: Up to 60 FPS (DMA-accelerated)
- **DMX Refresh Rate**: Up to 44 Hz (DMX512-A compliant)
- **RS485 Throughput**: Varies by baud rate (up to 115200 bps default)
- **CPU Usage**: Minimal due to interrupt/DMA-driven operation

## Troubleshooting

### Common Issues

1. **LEDs not responding**
   - Check pin connections and power supply
   - Verify LED count and grid configuration
   - Ensure proper voltage levels (3.3V vs 5V)

2. **DMX not transmitting**
   - Verify RS485 transceiver connections
   - Check termination resistors on DMX line
   - Confirm exactly 512 channels are being sent

3. **RS485 communication issues**
   - Check baud rate settings on both ends
   - Verify direction control pin operation
   - Ensure proper ground connections

### Build Issues

4. **CMake can't find Pico SDK**
   ```
   CMake Error: Pico SDK not found!
   ```
   - Ensure you've downloaded the Pico SDK as described in step 2
   - Verify the SDK path is correct
   - Try setting PICO_SDK_PATH environment variable

5. **Missing ARM toolchain**
   ```
   Could NOT find PkgConfig (missing: PKG_CONFIG_EXECUTABLE)
   ```
   - Install the ARM GCC toolchain as shown in prerequisites
   - On Windows, ensure the toolchain is in your PATH

6. **Git submodule errors**
   ```
   fatal: No url found for submodule path 'tinyusb'
   ```
   - Run `git submodule update --init` in the pico-sdk directory

### Debug Output

All examples include debug output via USB serial. Connect to the Pico's USB port and use a terminal at 115200 baud to see status messages.

## License

This project is provided as-is for educational and development purposes. Please ensure compliance with DMX512-A standards when using in professional lighting applications.

## Contributing

Contributions are welcome! Please ensure:
- Code follows the existing style and architecture
- DMX512 compliance is maintained (exactly 512 channels)
- All protocols remain functional
- Examples are updated as needed

## Support

For questions and support, please refer to the example code and inline documentation. The project is designed to be self-documenting with comprehensive examples for all major use cases.