#include "rs485_serial.h"
#include <cstring>
#include <cstdio>
#include <cstdarg>

// Static instance for interrupt handling
RS485Serial* RS485Serial::_instance = nullptr;

RS485Serial::RS485Serial(const Config& config) 
    : _config(config),
      _uart_irq(config.uart_instance == uart0 ? UART0_IRQ : UART1_IRQ),
      _initialized(false),
      _tx_buffer(nullptr),
      _tx_buffer_size(RS485_MAX_FRAME_SIZE),
      _tx_bytes_remaining(0),
      _tx_buffer_index(0),
      _status(Status::IDLE),
      _dma_channel(-1),
      _dma_available(false),
      _frames_sent(0),
      _bytes_sent(0),
      _transmission_errors(0),
      _last_transmission_time_us(0),
      _preamble_length(0),
      _postamble_length(0),
      _custom_frame_format(false),
      _pre_transmission_delay_us(RS485_TURNAROUND_TIME_US),
      _post_transmission_delay_us(RS485_TURNAROUND_TIME_US),
      _auto_direction_control(true) {
    
    _instance = this;
    memset(_preamble_data, 0, sizeof(_preamble_data));
    memset(_postamble_data, 0, sizeof(_postamble_data));
}

RS485Serial::~RS485Serial() {
    if (_initialized) {
        end();
    }
    
    if (_instance == this) {
        _instance = nullptr;
    }
}

RS485Serial::ReturnCode RS485Serial::begin() {
    if (_initialized) {
        return ReturnCode::SUCCESS;
    }

    // Validate configuration
    if (_config.data_pin >= NUM_BANK0_GPIOS) {
        return ReturnCode::ERROR_INVALID_PIN;
    }

    // Allocate transmission buffer
    _tx_buffer = (uint8_t*)malloc(_tx_buffer_size);
    if (_tx_buffer == nullptr) {
        return ReturnCode::ERROR_INVALID_PARAMETERS;
    }

    // Initialize UART
    uint actual_baud = uart_init(_config.uart_instance, _config.baud_rate);
    if (actual_baud == 0) {
        free(_tx_buffer);
        _tx_buffer = nullptr;
        return ReturnCode::ERROR_UART_INIT_FAILED;
    }

    // Configure UART
    if (!configure_uart()) {
        uart_deinit(_config.uart_instance);
        free(_tx_buffer);
        _tx_buffer = nullptr;
        return ReturnCode::ERROR_UART_INIT_FAILED;
    }

    // Set up GPIO for UART TX
    gpio_set_function(_config.data_pin, GPIO_FUNC_UART);

    // Configure RS485 direction control pin if specified
    if (_config.enable_pin != 0 && _config.enable_pin < NUM_BANK0_GPIOS) {
        gpio_init(_config.enable_pin);
        gpio_set_dir(_config.enable_pin, GPIO_OUT);
        disable_transmitter();  // Start in receive mode (for simplex, this is idle)
    }

    // Initialize DMA if requested
    if (_config.use_dma) {
        _dma_available = init_dma();
    }

    // Set up UART interrupt
    irq_set_exclusive_handler(_uart_irq, uart_irq_handler);
    irq_set_enabled(_uart_irq, true);
    uart_set_irq_enables(_config.uart_instance, false, true);  // TX interrupt only

    _initialized = true;
    _status = Status::IDLE;

    return ReturnCode::SUCCESS;
}

void RS485Serial::end() {
    if (!_initialized) {
        return;
    }

    // Wait for any ongoing transmission
    waitForCompletion(1000);  // 1 second timeout

    // Cleanup resources
    cleanup_dma();
    
    // Disable interrupts
    irq_set_enabled(_uart_irq, false);
    uart_set_irq_enables(_config.uart_instance, false, false);

    // Deinitialize UART
    uart_deinit(_config.uart_instance);

    // Free buffer
    if (_tx_buffer != nullptr) {
        free(_tx_buffer);
        _tx_buffer = nullptr;
    }

    _initialized = false;
    _status = Status::IDLE;
}

bool RS485Serial::configure_uart() {
    // Set UART format
    uart_set_format(_config.uart_instance, 
                   _config.data_bits, 
                   _config.stop_bits, 
                   _config.parity_enable ? 
                       (_config.parity_even ? UART_PARITY_EVEN : UART_PARITY_ODD) : 
                       UART_PARITY_NONE);

    // Disable hardware flow control
    uart_set_hw_flow(_config.uart_instance, false, false);

    return true;
}

bool RS485Serial::init_dma() {
    // Claim DMA channel
    _dma_channel = dma_claim_unused_channel(false);
    if (_dma_channel < 0) {
        return false;
    }

    // Configure DMA channel
    dma_channel_config config = dma_channel_get_default_config(_dma_channel);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_read_increment(&config, true);
    channel_config_set_write_increment(&config, false);
    channel_config_set_dreq(&config, uart_get_dreq(_config.uart_instance, true));

    // Set up DMA interrupt
    dma_channel_set_irq0_enabled(_dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    return true;
}

void RS485Serial::cleanup_dma() {
    if (_dma_channel >= 0) {
        dma_channel_abort(_dma_channel);
        dma_channel_set_irq0_enabled(_dma_channel, false);
        dma_channel_unclaim(_dma_channel);
        _dma_channel = -1;
    }
    _dma_available = false;
}

void RS485Serial::enable_transmitter() {
    if (_config.enable_pin != 0 && _config.enable_pin < NUM_BANK0_GPIOS) {
        gpio_put(_config.enable_pin, 1);  // Enable RS485 transmitter
        if (_pre_transmission_delay_us > 0) {
            busy_wait_us(_pre_transmission_delay_us);
        }
    }
}

void RS485Serial::disable_transmitter() {
    if (_config.enable_pin != 0 && _config.enable_pin < NUM_BANK0_GPIOS) {
        if (_post_transmission_delay_us > 0) {
            busy_wait_us(_post_transmission_delay_us);
        }
        gpio_put(_config.enable_pin, 0);  // Disable RS485 transmitter
    }
}

RS485Serial::ReturnCode RS485Serial::sendFrame(const uint8_t* data, uint16_t length, bool blocking) {
    if (!_initialized) {
        return ReturnCode::ERROR_NOT_INITIALIZED;
    }

    if (_status == Status::TRANSMITTING) {
        return ReturnCode::ERROR_TRANSMISSION_IN_PROGRESS;
    }

    if (data == nullptr || length == 0) {
        return ReturnCode::ERROR_INVALID_PARAMETERS;
    }

    // Check buffer size
    uint16_t total_length = length;
    if (_custom_frame_format) {
        total_length += _preamble_length + _postamble_length;
    }

    if (total_length > _tx_buffer_size) {
        return ReturnCode::ERROR_BUFFER_OVERFLOW;
    }

    // Prepare transmission buffer
    _tx_buffer_index = 0;
    uint16_t buffer_offset = 0;

    // Add preamble if configured
    if (_custom_frame_format && _preamble_length > 0) {
        memcpy(&_tx_buffer[buffer_offset], _preamble_data, _preamble_length);
        buffer_offset += _preamble_length;
    }

    // Add data
    memcpy(&_tx_buffer[buffer_offset], data, length);
    buffer_offset += length;

    // Add postamble if configured
    if (_custom_frame_format && _postamble_length > 0) {
        memcpy(&_tx_buffer[buffer_offset], _postamble_data, _postamble_length);
        buffer_offset += _postamble_length;
    }

    _tx_bytes_remaining = total_length;
    _status = Status::TRANSMITTING;
    _transmission_start = get_absolute_time();

    // Enable RS485 transmitter
    if (_auto_direction_control) {
        enable_transmitter();
    }

    if (_dma_available) {
        // Use DMA for transmission
        dma_channel_configure(_dma_channel,
                             dma_channel_get_default_config(_dma_channel),
                             &_config.uart_instance->dr,
                             _tx_buffer,
                             total_length,
                             true);  // Start transfer
    } else {
        // Use interrupt-driven transmission
        uart_set_irq_enables(_config.uart_instance, false, true);
        // Send first byte to trigger interrupt chain
        if (uart_is_writable(_config.uart_instance)) {
            uart_putc_raw(_config.uart_instance, _tx_buffer[_tx_buffer_index++]);
            _tx_bytes_remaining--;
        }
    }

    if (blocking) {
        bool completed = waitForCompletion(RS485_TX_TIMEOUT_MS);
        if (!completed) {
            abortTransmission();
            return ReturnCode::ERROR_TRANSMISSION_IN_PROGRESS;
        }
    }

    return ReturnCode::SUCCESS;
}

RS485Serial::ReturnCode RS485Serial::sendString(const char* str, bool blocking) {
    if (str == nullptr) {
        return ReturnCode::ERROR_INVALID_PARAMETERS;
    }

    uint16_t length = strlen(str);
    return sendFrame((const uint8_t*)str, length, blocking);
}

RS485Serial::ReturnCode RS485Serial::sendFormatted(const char* format, ...) {
    if (format == nullptr) {
        return ReturnCode::ERROR_INVALID_PARAMETERS;
    }

    // Use a temporary buffer for formatting
    char temp_buffer[512];  // Adjust size as needed
    
    va_list args;
    va_start(args, format);
    int length = vsnprintf(temp_buffer, sizeof(temp_buffer), format, args);
    va_end(args);

    if (length <= 0 || length >= (int)sizeof(temp_buffer)) {
        return ReturnCode::ERROR_INVALID_PARAMETERS;
    }

    return sendFrame((const uint8_t*)temp_buffer, length, false);
}

bool RS485Serial::waitForCompletion(uint32_t timeout_ms) {
    absolute_time_t start_time = get_absolute_time();

    while (_status == Status::TRANSMITTING) {
        if (timeout_ms > 0) {
            if (absolute_time_diff_us(start_time, get_absolute_time()) > (timeout_ms * 1000)) {
                return false;  // Timeout
            }
        }
        tight_loop_contents();
    }

    return true;
}

void RS485Serial::abortTransmission() {
    if (_status != Status::TRANSMITTING) {
        return;
    }

    // Stop DMA if active
    if (_dma_available && _dma_channel >= 0) {
        dma_channel_abort(_dma_channel);
    }

    // Disable UART interrupts
    uart_set_irq_enables(_config.uart_instance, false, false);

    // Disable transmitter
    if (_auto_direction_control) {
        disable_transmitter();
    }

    _status = Status::IDLE;
    _transmission_errors++;
}

bool RS485Serial::setBaudRate(uint32_t baud_rate) {
    if (!_initialized || _status == Status::TRANSMITTING) {
        return false;
    }

    uint actual_baud = uart_set_baudrate(_config.uart_instance, baud_rate);
    if (actual_baud > 0) {
        _config.baud_rate = actual_baud;
        return true;
    }

    return false;
}

bool RS485Serial::setBufferSize(uint16_t size) {
    if (_initialized || size == 0) {
        return false;  // Cannot change buffer size after initialization
    }

    _tx_buffer_size = size;
    return true;
}

void RS485Serial::getStatistics(uint32_t& frames_sent, uint32_t& bytes_sent, uint32_t& errors) const {
    frames_sent = _frames_sent;
    bytes_sent = _bytes_sent;
    errors = _transmission_errors;
}

void RS485Serial::resetStatistics() {
    _frames_sent = 0;
    _bytes_sent = 0;
    _transmission_errors = 0;
}

uint32_t RS485Serial::calculateTransmissionTime(uint16_t data_length) const {
    // Calculate bits per character
    uint bits_per_char = _config.data_bits + 1;  // +1 for start bit
    bits_per_char += _config.stop_bits;
    if (_config.parity_enable) {
        bits_per_char += 1;
    }

    // Calculate total bits
    uint32_t total_bits = data_length * bits_per_char;

    // Calculate time in microseconds
    uint32_t time_us = (total_bits * 1000000) / _config.baud_rate;

    return time_us;
}

void RS485Serial::setDirectionTiming(uint16_t pre_delay_us, uint16_t post_delay_us) {
    _pre_transmission_delay_us = pre_delay_us;
    _post_transmission_delay_us = post_delay_us;
}

void RS485Serial::setAutoDirectionControl(bool enable) {
    _auto_direction_control = enable;
}

RS485Serial::ReturnCode RS485Serial::sendFrameWithTiming(const uint8_t* data, uint16_t length, 
                                                       uint16_t inter_byte_delay_us, bool blocking) {
    // For now, implement as regular sendFrame
    // Inter-byte delay would require more complex state machine
    return sendFrame(data, length, blocking);
}

RS485Serial::ReturnCode RS485Serial::sendRepeatedFrame(const uint8_t* data, uint16_t length, 
                                                     uint16_t repeat_count, uint16_t inter_frame_delay_ms, 
                                                     bool blocking) {
    if (!_initialized || data == nullptr || length == 0 || repeat_count == 0) {
        return ReturnCode::ERROR_INVALID_PARAMETERS;
    }

    for (uint16_t i = 0; i < repeat_count; i++) {
        ReturnCode result = sendFrame(data, length, true);  // Wait for each frame
        if (result != ReturnCode::SUCCESS) {
            return result;
        }

        if (i < repeat_count - 1 && inter_frame_delay_ms > 0) {
            sleep_ms(inter_frame_delay_ms);
        }
    }

    return ReturnCode::SUCCESS;
}

void RS485Serial::setFrameFormat(const uint8_t* preamble_data, uint8_t preamble_length,
                                const uint8_t* postamble_data, uint8_t postamble_length) {
    _preamble_length = (preamble_length > 16) ? 16 : preamble_length;
    _postamble_length = (postamble_length > 16) ? 16 : postamble_length;

    if (preamble_data != nullptr && _preamble_length > 0) {
        memcpy(_preamble_data, preamble_data, _preamble_length);
    }

    if (postamble_data != nullptr && _postamble_length > 0) {
        memcpy(_postamble_data, postamble_data, _postamble_length);
    }

    _custom_frame_format = (_preamble_length > 0 || _postamble_length > 0);
}

void RS485Serial::uart_irq_handler() {
    if (_instance != nullptr) {
        _instance->handle_uart_interrupt();
    }
}

void RS485Serial::handle_uart_interrupt() {
    if (uart_is_writable(_config.uart_instance) && _tx_bytes_remaining > 0) {
        uart_putc_raw(_config.uart_instance, _tx_buffer[_tx_buffer_index++]);
        _tx_bytes_remaining--;

        if (_tx_bytes_remaining == 0) {
            // Transmission complete
            uart_set_irq_enables(_config.uart_instance, false, false);
            
            // Wait for UART to finish transmitting
            while (!uart_is_writable(_config.uart_instance)) {
                tight_loop_contents();
            }

            // Disable transmitter
            if (_auto_direction_control) {
                disable_transmitter();
            }

            _status = Status::IDLE;
            _frames_sent++;
            _bytes_sent += (_tx_buffer_index);
            _last_transmission_time_us = absolute_time_diff_us(_transmission_start, get_absolute_time());
        }
    }
}

void RS485Serial::dma_irq_handler() {
    if (_instance != nullptr) {
        _instance->handle_dma_complete();
    }
}

void RS485Serial::handle_dma_complete() {
    if (dma_channel_get_irq0_status(_dma_channel)) {
        dma_channel_acknowledge_irq0(_dma_channel);
        
        // Wait for UART to finish transmitting last byte
        while (!uart_is_writable(_config.uart_instance)) {
            tight_loop_contents();
        }

        // Disable transmitter
        if (_auto_direction_control) {
            disable_transmitter();
        }

        _status = Status::IDLE;
        _frames_sent++;
        _bytes_sent += _tx_bytes_remaining;  // DMA transmitted all bytes
        _tx_bytes_remaining = 0;
        _last_transmission_time_us = absolute_time_diff_us(_transmission_start, get_absolute_time());
    }
}

void RS485Serial::printStatus() const {
    printf("RS485 Serial Status:\n");
    printf("  Initialized: %s\n", _initialized ? "Yes" : "No");
    printf("  Data Pin: %u\n", _config.data_pin);
    printf("  Enable Pin: %u\n", _config.enable_pin);
    printf("  Baud Rate: %lu\n", _config.baud_rate);
    printf("  Status: ");
    
    switch (_status) {
        case Status::IDLE:
            printf("IDLE\n");
            break;
        case Status::TRANSMITTING:
            printf("TRANSMITTING (%u bytes remaining)\n", _tx_bytes_remaining);
            break;
        case Status::ERROR:
            printf("ERROR\n");
            break;
    }
    
    printf("  DMA Enabled: %s\n", _dma_available ? "Yes" : "No");
    printf("  Auto Direction Control: %s\n", _auto_direction_control ? "Yes" : "No");
}

void RS485Serial::printConfig() const {
    printf("RS485 Configuration:\n");
    printf("  Data Bits: %u\n", _config.data_bits);
    printf("  Stop Bits: %u\n", _config.stop_bits);
    printf("  Parity: %s\n", _config.parity_enable ? 
                           (_config.parity_even ? "Even" : "Odd") : "None");
    printf("  Buffer Size: %u bytes\n", _tx_buffer_size);
    printf("  Pre-TX Delay: %u us\n", _pre_transmission_delay_us);
    printf("  Post-TX Delay: %u us\n", _post_transmission_delay_us);
}

void RS485Serial::printStatistics() const {
    printf("RS485 Statistics:\n");
    printf("  Frames Sent: %lu\n", _frames_sent);
    printf("  Bytes Sent: %lu\n", _bytes_sent);
    printf("  Transmission Errors: %lu\n", _transmission_errors);
    printf("  Last Transmission Time: %lu us\n", _last_transmission_time_us);
}