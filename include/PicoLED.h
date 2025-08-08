#pragma once

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "../src/protocols/ws2812_driver.h"
#include "../src/protocols/dmx512_transmitter.h"
#include "../src/protocols/rs485_serial.h"
#include "../src/config/picoled_config.h"

/**
 * @brief Main PicoLED Protocol Bridge Class
 * 
 * Supports simultaneous operation of:
 * - WS2812/Serial LED Panel output (via PIO)
 * - DMX512 output (exactly 512 channels via RS485)
 * - RS485 Serial communication (simplex, variable length frames)
 */
class PicoLED {
public:
    // Protocol types
    enum class ProtocolType {
        WS2812_LED_PANEL,
        DMX512_OUTPUT,
        RS485_SERIAL
    };

    // Output pin configuration
    struct PinConfig {
        uint led_panel_pin;     // Pin for WS2812 LED panel output
        uint dmx512_pin;        // Pin for DMX512 output via RS485
        uint rs485_data_pin;    // Pin for RS485 serial data
        uint rs485_enable_pin;  // Pin for RS485 direction control (optional)
    };

    // LED panel configuration
    struct LEDConfig {
        uint num_pixels;
        uint grid_width;
        uint grid_height;
        PIO pio_instance;
        uint pio_sm;
    };

private:
    // Protocol handlers
    WS2812Driver* _led_driver;
    DMX512Transmitter* _dmx_transmitter;
    RS485Serial* _rs485_serial;

    // Configuration
    PinConfig _pins;
    LEDConfig _led_config;
    bool _initialized;

    // Data buffers
    uint32_t* _led_buffer;
    uint8_t _dmx_universe[DMX_UNIVERSE_SIZE];
    
    // Internal helper methods
    void init_hardware();
    void cleanup_resources();

public:
    /**
     * @brief Constructor
     * @param pins Pin configuration for all outputs
     * @param led_config LED panel configuration
     */
    PicoLED(const PinConfig& pins, const LEDConfig& led_config);

    /**
     * @brief Destructor
     */
    ~PicoLED();

    /**
     * @brief Initialize all protocol handlers
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Cleanup and shutdown all protocols
     */
    void end();

    // ===========================================
    // WS2812 LED Panel Methods
    // ===========================================

    /**
     * @brief Set color for specific LED by index
     */
    void setLEDColor(uint index, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Set color for LED at grid position
     */
    void setLEDColorXY(uint x, uint y, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Set all LEDs to same color
     */
    void setAllLEDs(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Clear all LEDs (set to black)
     */
    void clearAllLEDs();

    /**
     * @brief Update LED panel with current buffer
     */
    void updateLEDPanel();

    /**
     * @brief Convert DMX data to LED array
     * @param dmx_data Source DMX channel data
     * @param start_channel Starting DMX channel (1-based)
     * @param num_leds Number of LEDs to update
     */
    void dmxToLEDs(const uint8_t* dmx_data, uint16_t start_channel = 1, uint num_leds = 0);

    // ===========================================
    // DMX512 Output Methods (Exactly 512 channels)
    // ===========================================

    /**
     * @brief Set DMX channel value (1-512)
     */
    bool setDMXChannel(uint16_t channel, uint8_t value);

    /**
     * @brief Get DMX channel value (1-512)
     */
    uint8_t getDMXChannel(uint16_t channel) const;

    /**
     * @brief Set multiple DMX channels
     */
    bool setDMXChannelRange(uint16_t start_channel, const uint8_t* data, uint16_t length);

    /**
     * @brief Set entire DMX universe (512 channels)
     */
    void setDMXUniverse(const uint8_t* data);

    /**
     * @brief Convert LED data to DMX universe
     * @param start_channel Starting DMX channel for LED data
     */
    void ledsToDMX(uint16_t start_channel = 1);

    /**
     * @brief Clear all DMX channels to 0
     */
    void clearDMXUniverse();

    /**
     * @brief Transmit DMX512 universe (exactly 512 channels)
     */
    bool transmitDMX();

    /**
     * @brief Check if DMX transmission is in progress
     */
    bool isDMXBusy();

    /**
     * @brief Wait for DMX transmission to complete
     */
    void waitDMXCompletion();

    // ===========================================
    // RS485 Serial Communication Methods
    // ===========================================

    /**
     * @brief Send data frame via RS485 (simplex)
     * @param data Data buffer to send
     * @param length Number of bytes to send
     * @return true if transmission started successfully
     */
    bool sendRS485Frame(const uint8_t* data, uint16_t length);

    /**
     * @brief Send string via RS485
     */
    bool sendRS485String(const char* str);

    /**
     * @brief Check if RS485 transmission is busy
     */
    bool isRS485Busy();

    /**
     * @brief Wait for RS485 transmission to complete
     */
    void waitRS485Completion();

    /**
     * @brief Set RS485 baud rate
     */
    bool setRS485BaudRate(uint32_t baud_rate);

    // ===========================================
    // Status and Utility Methods
    // ===========================================

    /**
     * @brief Check if system is initialized
     */
    bool isInitialized() const { return _initialized; }

    /**
     * @brief Get LED buffer for direct access
     */
    uint32_t* getLEDBuffer() { return _led_buffer; }

    /**
     * @brief Get DMX universe buffer for direct access
     */
    uint8_t* getDMXBuffer() { return _dmx_universe; }

    /**
     * @brief Get LED configuration
     */
    const LEDConfig& getLEDConfig() const { return _led_config; }

    /**
     * @brief Get pin configuration
     */
    const PinConfig& getPinConfig() const { return _pins; }

    /**
     * @brief Update all protocols simultaneously
     * This method coordinates updates across all active protocols
     */
    void updateAll();

    /**
     * @brief Enable/disable specific protocol
     */
    void enableProtocol(ProtocolType protocol, bool enable);

    /**
     * @brief Check if protocol is enabled and ready
     */
    bool isProtocolReady(ProtocolType protocol) const;

    // Debug and diagnostic methods
    void printStatus();
    void printLEDState();
    void printDMXState();
};