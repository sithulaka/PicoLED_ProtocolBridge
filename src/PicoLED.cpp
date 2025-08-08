#include "../include/PicoLED.h"
#include <cstring>
#include <cstdio>

PicoLED::PicoLED(const PinConfig& pins, const LEDConfig& led_config) 
    : _led_driver(nullptr),
      _dmx_transmitter(nullptr),
      _rs485_serial(nullptr),
      _pins(pins),
      _led_config(led_config),
      _initialized(false),
      _led_buffer(nullptr) {
    
    // Initialize DMX universe to all zeros
    memset(_dmx_universe, 0, sizeof(_dmx_universe));
}

PicoLED::~PicoLED() {
    if (_initialized) {
        end();
    }
}

bool PicoLED::begin() {
    if (_initialized) {
        return true;
    }

    // Initialize hardware resources
    init_hardware();

    // Create WS2812 LED driver
    WS2812Driver::Config led_config = {
        .pio_instance = _led_config.pio_instance,
        .pio_sm = _led_config.pio_sm,
        .gpio_pin = _pins.led_panel_pin,
        .num_pixels = _led_config.num_pixels,
        .format = WS2812Driver::ColorFormat::GRB,  // WS2812 native format
        .use_dma = USE_DMA_FOR_LED_UPDATE
    };

    _led_driver = new WS2812Driver(led_config);
    if (!_led_driver || !_led_driver->begin()) {
        cleanup_resources();
        return false;
    }

    // Create DMX512 transmitter
    _dmx_transmitter = new DMX512Transmitter(_pins.dmx512_pin, uart1);
    if (!_dmx_transmitter || _dmx_transmitter->begin() != DMX512Transmitter::ReturnCode::SUCCESS) {
        cleanup_resources();
        return false;
    }

    // Create RS485 serial interface
    RS485Serial::Config rs485_config = {
        .data_pin = _pins.rs485_data_pin,
        .enable_pin = _pins.rs485_enable_pin,
        .uart_instance = uart0,
        .baud_rate = RS485_DEFAULT_BAUD,
        .data_bits = 8,
        .stop_bits = 1,
        .parity_enable = false,
        .parity_even = false,
        .use_dma = true
    };

    _rs485_serial = new RS485Serial(rs485_config);
    if (!_rs485_serial || _rs485_serial->begin() != RS485Serial::ReturnCode::SUCCESS) {
        cleanup_resources();
        return false;
    }

    // Allocate LED buffer
    _led_buffer = _led_driver->getPixelBuffer();

    _initialized = true;
    return true;
}

void PicoLED::end() {
    if (!_initialized) {
        return;
    }

    cleanup_resources();
    _initialized = false;
}

void PicoLED::init_hardware() {
    // Initialize clocks and basic hardware if needed
    // Most initialization is handled by individual drivers
}

void PicoLED::cleanup_resources() {
    if (_led_driver) {
        _led_driver->end();
        delete _led_driver;
        _led_driver = nullptr;
    }

    if (_dmx_transmitter) {
        _dmx_transmitter->end();
        delete _dmx_transmitter;
        _dmx_transmitter = nullptr;
    }

    if (_rs485_serial) {
        _rs485_serial->end();
        delete _rs485_serial;
        _rs485_serial = nullptr;
    }

    _led_buffer = nullptr;
}

// ===========================================
// WS2812 LED Panel Methods
// ===========================================

void PicoLED::setLEDColor(uint index, uint8_t r, uint8_t g, uint8_t b) {
    if (_led_driver) {
        _led_driver->setPixelColor(index, r, g, b);
    }
}

void PicoLED::setLEDColorXY(uint x, uint y, uint8_t r, uint8_t g, uint8_t b) {
    if (_led_driver && x < _led_config.grid_width && y < _led_config.grid_height) {
        uint index = y * _led_config.grid_width + x;
        _led_driver->setPixelColor(index, r, g, b);
    }
}

void PicoLED::setAllLEDs(uint8_t r, uint8_t g, uint8_t b) {
    if (_led_driver) {
        _led_driver->fill(r, g, b);
    }
}

void PicoLED::clearAllLEDs() {
    if (_led_driver) {
        _led_driver->clear();
    }
}

void PicoLED::updateLEDPanel() {
    if (_led_driver) {
        _led_driver->update(false);  // Non-blocking update
    }
}

void PicoLED::dmxToLEDs(const uint8_t* dmx_data, uint16_t start_channel, uint num_leds) {
    if (!_led_driver || !dmx_data) {
        return;
    }

    uint leds_to_update = (num_leds == 0) ? _led_config.num_pixels : num_leds;
    if (leds_to_update > _led_config.num_pixels) {
        leds_to_update = _led_config.num_pixels;
    }

    // Convert DMX data to LED colors (3 channels per LED: R, G, B)
    for (uint i = 0; i < leds_to_update; i++) {
        uint16_t dmx_offset = (start_channel - 1) + (i * 3);
        
        // Ensure we don't exceed DMX universe bounds
        if (dmx_offset + 2 < DMX_UNIVERSE_SIZE) {
            uint8_t r = dmx_data[dmx_offset];
            uint8_t g = dmx_data[dmx_offset + 1];
            uint8_t b = dmx_data[dmx_offset + 2];
            
            _led_driver->setPixelColor(i, r, g, b);
        }
    }
}

// ===========================================
// DMX512 Output Methods
// ===========================================

bool PicoLED::setDMXChannel(uint16_t channel, uint8_t value) {
    if (_dmx_transmitter) {
        bool success = _dmx_transmitter->setChannel(channel, value);
        if (success && channel <= DMX_UNIVERSE_SIZE) {
            _dmx_universe[channel - 1] = value;  // Keep local copy (0-based indexing)
        }
        return success;
    }
    return false;
}

uint8_t PicoLED::getDMXChannel(uint16_t channel) const {
    if (_dmx_transmitter && channel >= 1 && channel <= DMX_UNIVERSE_SIZE) {
        return _dmx_transmitter->getChannel(channel);
    }
    return 0;
}

bool PicoLED::setDMXChannelRange(uint16_t start_channel, const uint8_t* data, uint16_t length) {
    if (_dmx_transmitter) {
        bool success = _dmx_transmitter->setChannelRange(start_channel, data, length);
        if (success && start_channel >= 1 && start_channel <= DMX_UNIVERSE_SIZE) {
            // Update local copy
            uint16_t end_channel = start_channel + length - 1;
            if (end_channel > DMX_UNIVERSE_SIZE) {
                end_channel = DMX_UNIVERSE_SIZE;
            }
            uint16_t copy_length = end_channel - start_channel + 1;
            memcpy(&_dmx_universe[start_channel - 1], data, copy_length);
        }
        return success;
    }
    return false;
}

void PicoLED::setDMXUniverse(const uint8_t* data) {
    if (_dmx_transmitter && data) {
        _dmx_transmitter->setUniverse(data);
        memcpy(_dmx_universe, data, DMX_UNIVERSE_SIZE);
    }
}

void PicoLED::ledsToDMX(uint16_t start_channel) {
    if (!_led_driver || !_dmx_transmitter) {
        return;
    }

    uint32_t* led_buffer = _led_driver->getPixelBuffer();
    if (!led_buffer) {
        return;
    }

    // Convert LED data to DMX (3 channels per LED: R, G, B)
    uint num_pixels = _led_config.num_pixels;
    for (uint i = 0; i < num_pixels; i++) {
        uint16_t dmx_channel = start_channel + (i * 3);
        
        // Stop if we exceed DMX universe size
        if (dmx_channel + 2 > DMX_UNIVERSE_SIZE) {
            break;
        }

        // Extract RGB from LED buffer
        uint8_t r, g, b, w;
        _led_driver->nativeToColor(led_buffer[i], r, g, b, w);
        
        // Set DMX channels
        _dmx_transmitter->setChannel(dmx_channel, r);
        _dmx_transmitter->setChannel(dmx_channel + 1, g);
        _dmx_transmitter->setChannel(dmx_channel + 2, b);

        // Update local copy
        _dmx_universe[dmx_channel - 1] = r;
        _dmx_universe[dmx_channel] = g;
        _dmx_universe[dmx_channel + 1] = b;
    }
}

void PicoLED::clearDMXUniverse() {
    if (_dmx_transmitter) {
        _dmx_transmitter->clearUniverse();
        memset(_dmx_universe, 0, sizeof(_dmx_universe));
    }
}

bool PicoLED::transmitDMX() {
    if (_dmx_transmitter) {
        return _dmx_transmitter->transmit();
    }
    return false;
}

bool PicoLED::isDMXBusy() {
    if (_dmx_transmitter) {
        return _dmx_transmitter->isBusy();
    }
    return false;
}

void PicoLED::waitDMXCompletion() {
    if (_dmx_transmitter) {
        _dmx_transmitter->waitForCompletion(1000);  // 1 second timeout
    }
}

// ===========================================
// RS485 Serial Communication Methods
// ===========================================

bool PicoLED::sendRS485Frame(const uint8_t* data, uint16_t length) {
    if (_rs485_serial) {
        return _rs485_serial->sendFrame(data, length, false) == RS485Serial::ReturnCode::SUCCESS;
    }
    return false;
}

bool PicoLED::sendRS485String(const char* str) {
    if (_rs485_serial) {
        return _rs485_serial->sendString(str, false) == RS485Serial::ReturnCode::SUCCESS;
    }
    return false;
}

bool PicoLED::isRS485Busy() {
    if (_rs485_serial) {
        return _rs485_serial->isBusy();
    }
    return false;
}

void PicoLED::waitRS485Completion() {
    if (_rs485_serial) {
        _rs485_serial->waitForCompletion(1000);  // 1 second timeout
    }
}

bool PicoLED::setRS485BaudRate(uint32_t baud_rate) {
    if (_rs485_serial) {
        return _rs485_serial->setBaudRate(baud_rate);
    }
    return false;
}

// ===========================================
// Status and Utility Methods
// ===========================================

void PicoLED::updateAll() {
    // Update all protocols in coordinated manner
    if (_led_driver && !_led_driver->isBusy()) {
        _led_driver->update(false);
    }
    
    if (_dmx_transmitter && !_dmx_transmitter->isBusy()) {
        _dmx_transmitter->transmit();
    }
}

void PicoLED::enableProtocol(ProtocolType protocol, bool enable) {
    // This is a placeholder - actual implementation would enable/disable
    // specific protocol features based on the type
    switch (protocol) {
        case ProtocolType::WS2812_LED_PANEL:
            // Enable/disable LED driver
            break;
        case ProtocolType::DMX512_OUTPUT:
            // Enable/disable DMX transmitter
            break;
        case ProtocolType::RS485_SERIAL:
            // Enable/disable RS485 serial
            break;
    }
}

bool PicoLED::isProtocolReady(ProtocolType protocol) const {
    switch (protocol) {
        case ProtocolType::WS2812_LED_PANEL:
            return _led_driver && _led_driver->isInitialized();
        case ProtocolType::DMX512_OUTPUT:
            return _dmx_transmitter && _dmx_transmitter->isInitialized();
        case ProtocolType::RS485_SERIAL:
            return _rs485_serial && _rs485_serial->isInitialized();
        default:
            return false;
    }
}

void PicoLED::printStatus() {
    printf("=== PicoLED Protocol Bridge Status ===\n");
    printf("Initialized: %s\n", _initialized ? "Yes" : "No");
    printf("Pin Configuration:\n");
    printf("  LED Panel Pin: %u\n", _pins.led_panel_pin);
    printf("  DMX512 Pin: %u\n", _pins.dmx512_pin);
    printf("  RS485 Data Pin: %u\n", _pins.rs485_data_pin);
    printf("  RS485 Enable Pin: %u\n", _pins.rs485_enable_pin);
    
    printf("\nLED Configuration:\n");
    printf("  Pixels: %u (%ux%u grid)\n", _led_config.num_pixels, 
           _led_config.grid_width, _led_config.grid_height);
    
    printf("\nProtocol Status:\n");
    printf("  WS2812 LED Panel: %s\n", isProtocolReady(ProtocolType::WS2812_LED_PANEL) ? "Ready" : "Not Ready");
    printf("  DMX512 Output: %s\n", isProtocolReady(ProtocolType::DMX512_OUTPUT) ? "Ready" : "Not Ready");
    printf("  RS485 Serial: %s\n", isProtocolReady(ProtocolType::RS485_SERIAL) ? "Ready" : "Not Ready");
    
    // Print detailed status for each protocol
    if (_led_driver) {
        printf("\n");
        _led_driver->printStatus();
    }
    
    if (_dmx_transmitter) {
        printf("\n");
        _dmx_transmitter->printStatus();
    }
    
    if (_rs485_serial) {
        printf("\n");
        _rs485_serial->printStatus();
    }
}

void PicoLED::printLEDState() {
    if (_led_driver) {
        _led_driver->printPixelData(0, 8);  // Print first 8 pixels
    } else {
        printf("LED driver not initialized\n");
    }
}

void PicoLED::printDMXState() {
    if (_dmx_transmitter) {
        _dmx_transmitter->printFrame(1, 16);  // Print first 16 channels
    } else {
        printf("DMX transmitter not initialized\n");
    }
}