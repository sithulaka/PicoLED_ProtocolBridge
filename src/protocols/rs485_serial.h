#pragma once

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/dma.h"
#include "../config/picoled_config.h"

/**
 * @brief RS485 Serial Communication Driver
 * 
 * Implements simplex (transmit-only) RS485 communication
 * Supports variable-length frames with automatic direction control
 */
class RS485Serial {
public:
    enum class Status {
        IDLE,
        TRANSMITTING,
        ERROR
    };

    enum class ReturnCode {
        SUCCESS = 0,
        ERROR_INVALID_PIN,
        ERROR_UART_INIT_FAILED,
        ERROR_TRANSMISSION_IN_PROGRESS,
        ERROR_INVALID_PARAMETERS,
        ERROR_NOT_INITIALIZED,
        ERROR_BUFFER_OVERFLOW
    };

    struct Config {
        uint data_pin;          // UART TX pin for data
        uint enable_pin;        // Optional RS485 direction control pin (0 = not used)
        uart_inst_t* uart_instance;
        uint32_t baud_rate;
        uint8_t data_bits;      // 7 or 8
        uint8_t stop_bits;      // 1 or 2
        bool parity_enable;
        bool parity_even;       // true = even parity, false = odd parity
        bool use_dma;
    };

private:
    // Configuration
    Config _config;
    uint _uart_irq;
    bool _initialized;
    
    // Transmission buffer and state
    uint8_t* _tx_buffer;
    uint16_t _tx_buffer_size;
    volatile uint16_t _tx_bytes_remaining;
    volatile uint16_t _tx_buffer_index;
    volatile Status _status;
    
    // DMA support
    int _dma_channel;
    bool _dma_available;
    
    // Statistics
    uint32_t _frames_sent;
    uint32_t _bytes_sent;
    uint32_t _transmission_errors;
    
    // Timing
    absolute_time_t _transmission_start;
    uint32_t _last_transmission_time_us;
    
    // Internal methods
    bool configure_uart();
    bool init_dma();
    void cleanup_dma();
    void enable_transmitter();
    void disable_transmitter();
    void handle_uart_interrupt();
    void handle_dma_complete();
    void calculate_transmission_time(uint16_t data_length);
    
    // Static interrupt handlers
    static void uart_irq_handler();
    static void dma_irq_handler();
    static RS485Serial* _instance;

public:
    /**
     * @brief Constructor
     * @param config RS485 configuration
     */
    RS485Serial(const Config& config);

    /**
     * @brief Destructor
     */
    ~RS485Serial();

    /**
     * @brief Initialize RS485 interface
     * @return Success/error code
     */
    ReturnCode begin();

    /**
     * @brief Shutdown RS485 interface
     */
    void end();

    /**
     * @brief Send data frame (simplex transmission)
     * @param data Data buffer to transmit
     * @param length Number of bytes to transmit
     * @param blocking If true, wait for transmission to complete
     * @return Success/error code
     */
    ReturnCode sendFrame(const uint8_t* data, uint16_t length, bool blocking = false);

    /**
     * @brief Send string data
     * @param str Null-terminated string to transmit
     * @param blocking If true, wait for transmission to complete
     * @return Success/error code
     */
    ReturnCode sendString(const char* str, bool blocking = false);

    /**
     * @brief Send formatted string (like printf)
     * @param format Format string
     * @param ... Format arguments
     * @return Success/error code
     */
    ReturnCode sendFormatted(const char* format, ...);

    /**
     * @brief Check if transmission is in progress
     */
    bool isBusy() const { return _status == Status::TRANSMITTING; }

    /**
     * @brief Wait for current transmission to complete
     * @param timeout_ms Maximum time to wait in milliseconds (0 = infinite)
     * @return true if completed within timeout
     */
    bool waitForCompletion(uint32_t timeout_ms = 0);

    /**
     * @brief Abort current transmission
     */
    void abortTransmission();

    /**
     * @brief Set baud rate
     * @param baud_rate New baud rate
     * @return true if successful
     */
    bool setBaudRate(uint32_t baud_rate);

    /**
     * @brief Get current baud rate
     */
    uint32_t getBaudRate() const { return _config.baud_rate; }

    /**
     * @brief Set transmission buffer size
     * @param size Buffer size in bytes
     * @return true if successful
     */
    bool setBufferSize(uint16_t size);

    /**
     * @brief Get transmission buffer size
     */
    uint16_t getBufferSize() const { return _tx_buffer_size; }

    /**
     * @brief Check if interface is initialized
     */
    bool isInitialized() const { return _initialized; }

    /**
     * @brief Get current status
     */
    Status getStatus() const { return _status; }

    /**
     * @brief Get configuration
     */
    const Config& getConfig() const { return _config; }

    /**
     * @brief Get transmission statistics
     * @param frames_sent Total frames transmitted
     * @param bytes_sent Total bytes transmitted
     * @param errors Total transmission errors
     */
    void getStatistics(uint32_t& frames_sent, uint32_t& bytes_sent, uint32_t& errors) const;

    /**
     * @brief Reset transmission statistics
     */
    void resetStatistics();

    /**
     * @brief Calculate transmission time for given data length
     * @param data_length Number of bytes to transmit
     * @return Transmission time in microseconds
     */
    uint32_t calculateTransmissionTime(uint16_t data_length) const;

    /**
     * @brief Get time of last transmission
     * @return Time in microseconds since last transmission started
     */
    uint32_t getLastTransmissionTime() const { return _last_transmission_time_us; }

    /**
     * @brief Set direction control timing
     * @param pre_delay_us Delay before transmission starts (microseconds)
     * @param post_delay_us Delay after transmission ends (microseconds)
     */
    void setDirectionTiming(uint16_t pre_delay_us, uint16_t post_delay_us);

    /**
     * @brief Enable/disable automatic direction control
     * @param enable If true, automatically control RS485 direction pin
     */
    void setAutoDirectionControl(bool enable);

    // Advanced features

    /**
     * @brief Send frame with custom timing
     * @param data Data to transmit
     * @param length Data length
     * @param inter_byte_delay_us Delay between bytes (microseconds)
     * @param blocking Wait for completion
     * @return Success/error code
     */
    ReturnCode sendFrameWithTiming(const uint8_t* data, uint16_t length, 
                                 uint16_t inter_byte_delay_us, bool blocking = false);

    /**
     * @brief Send repeated frame
     * @param data Data to transmit
     * @param length Data length
     * @param repeat_count Number of times to repeat
     * @param inter_frame_delay_ms Delay between frames (milliseconds)
     * @param blocking Wait for all transmissions to complete
     * @return Success/error code
     */
    ReturnCode sendRepeatedFrame(const uint8_t* data, uint16_t length, 
                               uint16_t repeat_count, uint16_t inter_frame_delay_ms, 
                               bool blocking = false);

    /**
     * @brief Configure custom frame format
     * @param preamble_data Preamble bytes (can be nullptr)
     * @param preamble_length Preamble length
     * @param postamble_data Postamble bytes (can be nullptr)
     * @param postamble_length Postamble length
     */
    void setFrameFormat(const uint8_t* preamble_data, uint8_t preamble_length,
                       const uint8_t* postamble_data, uint8_t postamble_length);

    // Debug and diagnostic methods
    void printStatus() const;
    void printConfig() const;
    void printStatistics() const;

private:
    // Frame format customization
    uint8_t _preamble_data[16];
    uint8_t _preamble_length;
    uint8_t _postamble_data[16];
    uint8_t _postamble_length;
    bool _custom_frame_format;
    
    // Direction control timing
    uint16_t _pre_transmission_delay_us;
    uint16_t _post_transmission_delay_us;
    bool _auto_direction_control;
};