#pragma once

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "../config/picoled_config.h"

/**
 * @brief DMX512 Transmitter Class
 * 
 * Implements DMX512-A standard with exactly 512 channels
 * Designed for RS485 output with proper timing and frame structure
 */
class DMX512Transmitter {
public:
    enum class Status {
        IDLE,
        TRANSMITTING_BREAK,
        TRANSMITTING_MAB,  // Mark After Break
        TRANSMITTING_DATA,
        ERROR
    };

    enum class ReturnCode {
        SUCCESS = 0,
        ERROR_INVALID_PIN,
        ERROR_UART_INIT_FAILED,
        ERROR_INVALID_CHANNEL,
        ERROR_TRANSMISSION_IN_PROGRESS,
        ERROR_NOT_INITIALIZED
    };

private:
    // Hardware configuration
    uint _gpio_pin;
    uart_inst_t* _uart_instance;
    uint _uart_irq;
    
    // DMX512 data buffer (513 bytes: start code + 512 channels)
    uint8_t _dmx_frame[DMX_UNIVERSE_SIZE + 1];
    
    // Transmission state
    volatile Status _status;
    volatile uint16_t _current_byte_index;
    bool _initialized;
    bool _continuous_mode;
    
    // Timing control
    absolute_time_t _break_start_time;
    absolute_time_t _mab_start_time;
    
    // Statistics
    uint32_t _frame_count;
    uint32_t _error_count;
    
    // Internal methods
    void configure_uart();
    void start_break();
    void start_mab();
    void start_data_transmission();
    void handle_uart_interrupt();
    static void uart_irq_handler();
    
    // Timing helpers
    void delay_microseconds(uint32_t us);
    bool is_break_complete() const;
    bool is_mab_complete() const;
    
public:
    /**
     * @brief Constructor
     * @param gpio_pin GPIO pin for DMX output (connected to RS485 driver)
     * @param uart_instance UART instance to use (uart0 or uart1)
     */
    DMX512Transmitter(uint gpio_pin, uart_inst_t* uart_instance = uart1);

    /**
     * @brief Destructor
     */
    ~DMX512Transmitter();

    /**
     * @brief Initialize DMX transmitter
     * @param baud_rate DMX baud rate (standard is 250000)
     * @return Success/error code
     */
    ReturnCode begin(uint32_t baud_rate = 250000);

    /**
     * @brief Shutdown transmitter
     */
    void end();

    /**
     * @brief Set DMX channel value (1-512)
     * @param channel Channel number (1-512)
     * @param value Channel value (0-255)
     * @return true if successful
     */
    bool setChannel(uint16_t channel, uint8_t value);

    /**
     * @brief Get DMX channel value (1-512)
     * @param channel Channel number (1-512)
     * @return Channel value (0 if invalid channel)
     */
    uint8_t getChannel(uint16_t channel) const;

    /**
     * @brief Set multiple channels starting from start_channel
     * @param start_channel Starting channel (1-512)
     * @param data Source data array
     * @param length Number of bytes to copy
     * @return true if successful
     */
    bool setChannelRange(uint16_t start_channel, const uint8_t* data, uint16_t length);

    /**
     * @brief Set entire DMX universe (exactly 512 channels)
     * @param data Source data array (must be at least 512 bytes)
     */
    void setUniverse(const uint8_t* data);

    /**
     * @brief Clear all channels to 0
     */
    void clearUniverse();

    /**
     * @brief Transmit current DMX frame (exactly 512 channels + start code)
     * @return true if transmission started successfully
     */
    bool transmit();

    /**
     * @brief Enable/disable continuous transmission mode
     * @param enable If true, automatically retransmit at DMX refresh rate
     */
    void setContinuousMode(bool enable);

    /**
     * @brief Check if transmission is currently in progress
     */
    bool isBusy() const { return _status != Status::IDLE; }

    /**
     * @brief Wait for current transmission to complete
     * @param timeout_ms Maximum time to wait in milliseconds (0 = infinite)
     * @return true if completed within timeout
     */
    bool waitForCompletion(uint32_t timeout_ms = 0);

    /**
     * @brief Get current transmission status
     */
    Status getStatus() const { return _status; }

    /**
     * @brief Check if transmitter is initialized
     */
    bool isInitialized() const { return _initialized; }

    /**
     * @brief Get GPIO pin number
     */
    uint getGpioPin() const { return _gpio_pin; }

    /**
     * @brief Get frame transmission statistics
     * @param frame_count Total frames transmitted
     * @param error_count Total transmission errors
     */
    void getStatistics(uint32_t& frame_count, uint32_t& error_count) const;

    /**
     * @brief Reset transmission statistics
     */
    void resetStatistics();

    /**
     * @brief Get direct access to DMX frame buffer
     * @return Pointer to 513-byte buffer (start code + 512 channels)
     */
    uint8_t* getFrameBuffer() { return _dmx_frame; }

    /**
     * @brief Validate DMX frame integrity
     * @return true if frame is valid
     */
    bool validateFrame() const;

    /**
     * @brief Set custom start code (default is 0x00 for dimmer data)
     * @param start_code Start code value
     */
    void setStartCode(uint8_t start_code) { _dmx_frame[0] = start_code; }

    /**
     * @brief Get current start code
     */
    uint8_t getStartCode() const { return _dmx_frame[0]; }

    // Debug and diagnostic methods
    void printStatus() const;
    void printFrame(uint16_t start_channel = 1, uint16_t count = 16) const;
    
    // Static instance for interrupt handling
    static DMX512Transmitter* _instance;
};