# Documentation Index - PicoLED Protocol Bridge

This folder contains comprehensive technical documentation for the PicoLED Protocol Bridge project, covering in-depth analysis of embedded systems concepts, hardware interfaces, and protocol implementations.

## 📁 Documentation Files

### **Core Library Analysis**
- **[README_PicoLED_Analysis.md](README_PicoLED_Analysis.md)** - Complete breakdown of the PicoLED.cpp class implementation, including memory management, color encoding, DMX integration, and hardware interface details.

### **Hardware Protocol Explanations**
- **[README_WS2812_32bit_Explanation.md](README_WS2812_32bit_Explanation.md)** - Detailed explanation of why WS2812 LEDs use 32-bit transfers despite being 24-bit devices, covering PIO auto-pull mechanisms and bit alignment.

- **[README_Pico_DMX_Library.md](README_Pico_DMX_Library.md)** - Comprehensive analysis of the DMX512 protocol implementation, including serial framing, timing calculations, and integration examples.

- **[README_DMX_PIO_Assembly.md](README_DMX_PIO_Assembly.md)** - Low-level analysis of DMX PIO assembly code with complete instruction breakdown, register operations, and timing verification.

### **Programming Concepts**
- **[README_Volatile_Keyword.md](README_Volatile_Keyword.md)** - Essential guide to the `volatile` keyword in C++, covering compiler optimizations, use cases, and common pitfalls.

- **[README_Volatile_Multicore.md](README_Volatile_Multicore.md)** - Advanced multi-core programming concepts for Raspberry Pi Pico, including synchronization primitives, race conditions, and best practices.

## 🎯 Key Topics Covered

### **Embedded Systems Programming**
- PIO (Programmable I/O) state machine programming
- DMA integration and hardware acceleration
- Multi-core synchronization on ARM Cortex-M0+
- Real-time protocol implementation

### **Professional Lighting Protocols**
- DMX512 serial communication standard
- 11-bit framing and timing requirements
- RS-485 electrical interface considerations
- Frame rate and latency analysis

### **LED Control Technology**
- WS2812 RGB LED protocol implementation
- Color encoding and bit manipulation
- Grid/matrix addressing schemes
- Performance optimization techniques

### **C++ Advanced Concepts**
- Memory management in embedded systems
- Volatile keyword and compiler optimizations
- Hardware abstraction layer design
- Thread safety and atomic operations

## 🔧 Technical Specifications

### **Timing Analysis**
- **DMX Baud Rate**: 250,000 bps
- **WS2812 Timing**: 800kHz data rate
- **Frame Rates**: 44Hz DMX, variable LED refresh
- **Precision**: ±1μs timing accuracy via PIO

### **Hardware Requirements**
- **Raspberry Pi Pico**: RP2040 microcontroller
- **PIO Blocks**: 2 available (pio0, pio1)
- **DMA Channels**: Hardware-accelerated data transfer
- **GPIO Pins**: Configurable for DMX and LED interfaces

### **Memory Usage**
- **LED Buffer**: 4 bytes per LED (uint32_t)
- **DMX Buffer**: 513 bytes (start code + 512 channels)
- **PIO FIFO**: 8-level deep hardware buffer
- **Optimization**: Efficient for Pico's 264KB RAM

## 🚀 Getting Started

1. **Read Core Analysis**: Start with [README_PicoLED_Analysis.md](README_PicoLED_Analysis.md) for overall architecture understanding
2. **Understand Protocols**: Review DMX and WS2812 documentation for protocol details
3. **Study Assembly**: Examine [README_DMX_PIO_Assembly.md](README_DMX_PIO_Assembly.md) for low-level implementation
4. **Learn Concurrency**: Check volatile and multi-core documentation for advanced topics

## 📚 Additional Resources

### **Related Files in Project**
- `src/PicoLED.cpp` - Main LED control implementation
- `include/PicoLED.h` - Class interface and declarations
- `Pico-DMX/` - DMX protocol library
- `extras/ws2812.pio` - WS2812 PIO assembly program

### **External Documentation**
- [RP2040 Datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf)
- [Pico SDK Documentation](https://raspberrypi.github.io/pico-sdk-doxygen/)
- [DMX512 Standard](https://tsp.esta.org/tsp/documents/docs/ANSI-ESTA_E1-11_2008R2018.pdf)
- [WS2812 Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812.pdf)

---

**Created**: June 28, 2025  
**Project**: PicoLED Protocol Bridge  
**Version**: Technical Documentation v1.0
