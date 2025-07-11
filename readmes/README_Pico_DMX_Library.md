# Pico-DMX Library - Deep Dive Analysis

## **Overview**

The Pico-DMX library provides **hardware-accelerated DMX512 protocol support** using the Raspberry Pi Pico's PIO (Programmable I/O) system. It enables your Pico to act as either a **DMX controller** (output) or **DMX receiver** (input) for professional lighting control.

## **DMX512 Protocol Fundamentals**

### **What is DMX512?**
- **Digital Multiplex 512**: Professional lighting control standard
- **512 channels**: Each channel = 8-bit value (0-255)
- **Serial Protocol**: RS-485 electrical standard at 250kbps
- **Frame Structure**: Start + 512 data bytes + stop bits

### **DMX Frame Format:**
```
[BREAK][MAB][START][CH1][CH2]...[CH512][STOP]
```
- **BREAK**: 88μs+ LOW signal (marks frame start)
- **MAB**: Mark After Break (8μs+ HIGH)
- **START**: Start code (usually 0x00)
- **CH1-512**: Channel data (0-255 each)

### **Serial Framing Details**

**Every DMX channel value is transmitted as a single byte, but on the wire, each byte is framed with additional bits for reliable serial communication.**

### **For Each Channel (and the Start Code):**
- **1 start bit** (always 0)
- **8 data bits** (the actual channel value, 0–255, least significant bit first)
- **2 stop bits** (always 1)

**Total:** **11 bits** are sent for each channel value (or for the start code byte).

### **Why 11 Bits?**
- DMX uses asynchronous serial communication (like classic RS-232).
- The start and stop bits help the receiver know where each byte begins and ends, ensuring proper synchronization.

### **Example:**
If you want to send the value 0x7F (127) to DMX channel 1, the bits on the wire would be:

| Start Bit | Data Bits (LSB first) | Stop Bits |
|-----------|-----------------------|-----------|
| 0         | 1 1 1 1 1 1 1 0       | 1 1       |

So, the receiver sees 11 bits for each channel value.

### **Timing Calculation:**
- **11 bits per channel** × **4μs per bit** = **44μs per channel**
- **513 channels total** (start code + 512 data) × **44μs** = **22.572ms per frame**
- **Frame rate**: 1 ÷ 22.572ms = **44.3 Hz**

## **Library Architecture**

### **Core Components:**

1. **DmxOutput.cpp/.h**: DMX transmission (controller mode)
2. **DmxInput.cpp/.h**: DMX reception (receiver mode)  
3. **PIO Programs**: Hardware-level protocol implementation
4. **Integration**: Works with your PicoLED class

## **Integration with PicoLED Project**

### **Current Integration Point**

Your `PicoLED::DmxArray_to_GRBArray_Converter()` method already accepts DMX data:

```cpp
void PicoLED::DmxArray_to_GRBArray_Converter(const uint8_t* DmxArray) {
    uint index = 0;
    uint max_iterations = (NUM_CHANNELS / 3) * 3;
    
    for (uint i = 0; i < max_iterations; i+=3) {
        pixel_grb = urgb_u32(DmxArray[i], DmxArray[i+1], DmxArray[i+2]);
        if (index < num_pixels) {
            led_array[index] = pixel_grb;
        }
        index++;
    }
}
```

### **Enhanced Integration Example**

```cpp
#include "PicoLED.h"
#include "DmxInput.h"
#include "DmxOutput.h"

class DMXLEDBridge {
private:
    PicoLED* leds;
    DmxInput* dmx_in;
    DmxOutput* dmx_out;
    
public:
    DMXLEDBridge(PIO pio0, PIO pio1, uint led_pin, uint dmx_pin) {
        // Initialize LED controller
        leds = new PicoLED(pio0, 0, NUM_PIXELS, GRID_WIDTH);
        
        // Initialize DMX input
        dmx_in = new DmxInput(pio1, 0, dmx_pin);
        
        // Optional: DMX output for daisy-chaining
        dmx_out = new DmxOutput(pio1, 1, dmx_pin + 1);
    }
    
    void processFrame() {
        if (dmx_in->isFrameReady()) {
            // Get DMX data
            uint8_t dmx_data[512];
            for (int i = 1; i <= 512; i++) {
                dmx_data[i-1] = dmx_in->getChannel(i);
            }
            
            // Convert to LED colors
            leds->DmxArray_to_GRBArray_Converter(dmx_data);
            
            // Update physical LEDs
            leds->push_array();
            
            // Optional: Forward DMX data
            forwardDMX(dmx_data);
        }
    }
    
    void forwardDMX(uint8_t* data) {
        // Forward DMX to next device in chain
        for (int i = 1; i <= 512; i++) {
            dmx_out->setChannel(i, data[i-1]);
        }
        dmx_out->sendFrame();
    }
};
```

## **Advanced Usage Scenarios**

### **1. DMX-to-LED Matrix Bridge**
```cpp
void mapDMXtoMatrix() {
    // Map DMX channels to specific LED positions
    for (uint y = 1; y <= 8; y++) {
        for (uint x = 1; x <= 8; x++) {
            uint dmx_base = ((y-1) * 8 + (x-1)) * 3 + 1;
            uint8_t r = dmx_in->getChannel(dmx_base);
            uint8_t g = dmx_in->getChannel(dmx_base + 1);
            uint8_t b = dmx_in->getChannel(dmx_base + 2);
            
            leds->set_XY(x, y, r, g, b);
        }
    }
    leds->push_array();
}
```

### **2. DMX Fixture Simulation**
```cpp
class DMXFixture {
private:
    uint16_t start_channel;
    PicoLED* leds;
    
public:
    void setDimmer(uint8_t value) {
        // Channel 1: Master dimmer
        uint8_t dimmer = dmx_in->getChannel(start_channel);
        // Apply dimmer to all colors
    }
    
    void setColor() {
        // Channels 2,3,4: RGB
        uint8_t r = dmx_in->getChannel(start_channel + 1);
        uint8_t g = dmx_in->getChannel(start_channel + 2);
        uint8_t b = dmx_in->getChannel(start_channel + 3);
        leds->change_all_color(r, g, b);
    }
    
    void setStrobe() {
        // Channel 5: Strobe effect
        uint8_t strobe = dmx_in->getChannel(start_channel + 4);
        if (strobe > 10) {
            // Implement strobe logic
        }
    }
};
```

## **Performance Characteristics**

### **Hardware Advantages:**
- **PIO-based**: No CPU overhead for protocol timing
- **Concurrent**: DMX and LED operations run simultaneously
- **Precise**: ±1μs timing accuracy
- **Scalable**: Multiple universes on different PIO blocks

### **Throughput:**
- **DMX Rate**: 44Hz refresh rate (22.7ms per frame)
- **LED Update**: Can update faster than DMX input
- **Latency**: <1ms from DMX receive to LED update

## **Configuration Options**

### **CMake Integration**
```cmake
# Add to your CMakeLists.txt
include(Pico-DMX/interfaceLibForPicoSDK.cmake)
target_link_libraries(your_project pico-dmx)
```

### **Pin Configuration:**
```cpp
// DMX typically uses GPIO pins with RS-485 transceivers
#define DMX_INPUT_PIN  16  // Connect to RS-485 receiver output
#define DMX_OUTPUT_PIN 17  // Connect to RS-485 driver input
#define LED_DATA_PIN   2   // WS2812 data line
```

## **Real-World Applications**

1. **Stage Lighting**: Convert DMX lighting consoles to LED strips
2. **Architectural Lighting**: DMX-controlled building illumination  
3. **Event Production**: Sync LED displays with lighting desks
4. **Art Installations**: Interactive lighting responsive to DMX
5. **Broadcast**: LED walls controlled by lighting operators

This library transforms your Pico into a professional lighting interface, bridging the gap between traditional DMX lighting systems and modern LED technology. The PIO-based implementation ensures reliable, precise timing that meets professional lighting standards.
