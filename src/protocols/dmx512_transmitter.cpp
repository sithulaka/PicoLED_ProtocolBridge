#include "dmx512_transmitter.h"
#include <cstring>
#include <cstdio>

// Static instance for interrupt handling
DMX512Transmitter* DMX512Transmitter::_instance = nullptr;

DMX512Transmitter::DMX512Transmitter(uint gpio_pin, uart_inst_t* uart_instance) 
    : _gpio_pin(gpio_pin), 
      _uart_instance(uart_instance),
      _uart_irq(uart_instance == uart0 ? UART0_IRQ : UART1_IRQ),
      _status(Status::IDLE),
      _current_byte_index(0),
      _initialized(false),
      _continuous_mode(false),
      _frame_count(0),
      _error_count(0) {
    
    // Initialize DMX frame with start code and all channels to 0
    memset(_dmx_frame, 0, sizeof(_dmx_frame));
    _dmx_frame[0] = DMX_START_CODE;
    
    // Set static instance for interrupt handling
    _instance = this;
}

DMX512Transmitter::~DMX512Transmitter() {
    if (_initialized) {
        end();
    }
    if (_instance == this) {
        _instance = nullptr;
    }
}

DMX512Transmitter::ReturnCode DMX512Transmitter::begin(uint32_t baud_rate) {
    if (_initialized) {
        return ReturnCode::SUCCESS;
    }

    // Validate GPIO pin
    if (_gpio_pin >= NUM_BANK0_GPIOS) {
        return ReturnCode::ERROR_INVALID_PIN;
    }

    // Initialize UART
    uint actual_baud = uart_init(_uart_instance, baud_rate);
    if (actual_baud == 0) {
        return ReturnCode::ERROR_UART_INIT_FAILED;
    }

    // Configure UART for DMX512
    configure_uart();

    // Set up GPIO for UART TX
    gpio_set_function(_gpio_pin, GPIO_FUNC_UART);
    
    // Set up interrupt handler
    irq_set_exclusive_handler(_uart_irq, uart_irq_handler);
    irq_set_enabled(_uart_irq, true);

    _initialized = true;
    _status = Status::IDLE;

    return ReturnCode::SUCCESS;
}

void DMX512Transmitter::configure_uart() {
    // DMX512 configuration: 8 data bits, 2 stop bits, no parity
    uart_set_format(_uart_instance, 8, 2, UART_PARITY_NONE);
    
    // Configure hardware flow control (disabled for DMX)
    uart_set_hw_flow(_uart_instance, false, false);
    
    // Enable TX interrupt
    uart_set_irq_enables(_uart_instance, false, true);  // RX disabled, TX enabled
}

void DMX512Transmitter::end() {
    if (!_initialized) {
        return;
    }

    // Wait for any ongoing transmission to complete
    waitForCompletion(1000);  // 1 second timeout

    // Disable interrupt
    irq_set_enabled(_uart_irq, false);
    
    // Deinitialize UART
    uart_deinit(_uart_instance);
    
    _initialized = false;
    _status = Status::IDLE;
}

bool DMX512Transmitter::setChannel(uint16_t channel, uint8_t value) {
    if (channel < 1 || channel > DMX_UNIVERSE_SIZE) {
        return false;
    }
    
    // Channels are 1-based, array is 0-based (index 0 is start code)
    _dmx_frame[channel] = value;
    return true;
}

uint8_t DMX512Transmitter::getChannel(uint16_t channel) const {
    if (channel < 1 || channel > DMX_UNIVERSE_SIZE) {
        return 0;
    }
    
    return _dmx_frame[channel];
}

bool DMX512Transmitter::setChannelRange(uint16_t start_channel, const uint8_t* data, uint16_t length) {
    if (start_channel < 1 || start_channel > DMX_UNIVERSE_SIZE || 
        start_channel + length - 1 > DMX_UNIVERSE_SIZE || data == nullptr) {
        return false;
    }
    
    memcpy(&_dmx_frame[start_channel], data, length);
    return true;
}

void DMX512Transmitter::setUniverse(const uint8_t* data) {
    if (data == nullptr) {
        return;
    }
    
    // Copy exactly 512 channels (preserve start code at index 0)
    memcpy(&_dmx_frame[1], data, DMX_UNIVERSE_SIZE);
}

void DMX512Transmitter::clearUniverse() {
    // Clear all channels to 0 (preserve start code)
    memset(&_dmx_frame[1], 0, DMX_UNIVERSE_SIZE);
}

bool DMX512Transmitter::transmit() {
    if (!_initialized) {
        return false;
    }

    if (_status != Status::IDLE) {
        return false;  // Transmission already in progress
    }

    // Start DMX transmission sequence
    start_break();
    return true;
}

void DMX512Transmitter::setContinuousMode(bool enable) {
    _continuous_mode = enable;
    
    if (enable && _status == Status::IDLE) {
        transmit();  // Start transmission immediately
    }
}

void DMX512Transmitter::start_break() {
    _status = Status::TRANSMITTING_BREAK;
    
    // Configure GPIO as output for break
    gpio_init(_gpio_pin);
    gpio_set_dir(_gpio_pin, GPIO_OUT);
    gpio_put(_gpio_pin, 0);  // Pull low for break
    
    _break_start_time = get_absolute_time();
    
    // Set up timer for break duration
    busy_wait_us(DMX_BREAK_TIME_US);
    
    start_mab();
}

void DMX512Transmitter::start_mab() {
    _status = Status::TRANSMITTING_MAB;
    
    // Pull high for Mark After Break
    gpio_put(_gpio_pin, 1);
    
    _mab_start_time = get_absolute_time();
    
    // Wait for MAB duration
    busy_wait_us(DMX_MARK_TIME_US);
    
    start_data_transmission();
}

void DMX512Transmitter::start_data_transmission() {
    _status = Status::TRANSMITTING_DATA;
    _current_byte_index = 0;
    
    // Reconfigure pin for UART
    gpio_set_function(_gpio_pin, GPIO_FUNC_UART);
    
    // Enable UART TX interrupt
    uart_set_irq_enables(_uart_instance, false, true);
    
    // Send first byte (start code)
    uart_putc_raw(_uart_instance, _dmx_frame[0]);
    _current_byte_index = 1;
}

void DMX512Transmitter::uart_irq_handler() {
    if (_instance != nullptr) {
        _instance->handle_uart_interrupt();
    }
}

void DMX512Transmitter::handle_uart_interrupt() {
    // Check if TX FIFO has space and we have more data to send
    if (uart_is_writable(_uart_instance) && _current_byte_index <= DMX_UNIVERSE_SIZE) {
        if (_current_byte_index <= DMX_UNIVERSE_SIZE) {
            // Send next byte
            uart_putc_raw(_uart_instance, _dmx_frame[_current_byte_index]);
            _current_byte_index++;
        } else {
            // Transmission complete
            uart_set_irq_enables(_uart_instance, false, false);  // Disable TX interrupt
            
            _status = Status::IDLE;
            _frame_count++;
            
            // If continuous mode is enabled, start next frame after a short delay
            if (_continuous_mode) {
                // Add small delay between frames to maintain proper DMX timing
                busy_wait_us(1000);  // 1ms delay
                start_break();
            }
        }
    }
}

bool DMX512Transmitter::waitForCompletion(uint32_t timeout_ms) {
    absolute_time_t start_time = get_absolute_time();
    
    while (_status != Status::IDLE) {
        if (timeout_ms > 0) {
            if (absolute_time_diff_us(start_time, get_absolute_time()) > (timeout_ms * 1000)) {
                return false;  // Timeout
            }
        }
        tight_loop_contents();
    }
    
    return true;
}

void DMX512Transmitter::getStatistics(uint32_t& frame_count, uint32_t& error_count) const {
    frame_count = _frame_count;
    error_count = _error_count;
}

void DMX512Transmitter::resetStatistics() {
    _frame_count = 0;
    _error_count = 0;
}

bool DMX512Transmitter::validateFrame() const {
    // Basic frame validation
    if (_dmx_frame[0] != DMX_START_CODE) {
        return false;
    }
    
    // Could add more validation checks here
    return true;
}

void DMX512Transmitter::delay_microseconds(uint32_t us) {
    busy_wait_us(us);
}

bool DMX512Transmitter::is_break_complete() const {
    return absolute_time_diff_us(_break_start_time, get_absolute_time()) >= DMX_BREAK_TIME_US;
}

bool DMX512Transmitter::is_mab_complete() const {
    return absolute_time_diff_us(_mab_start_time, get_absolute_time()) >= DMX_MARK_TIME_US;
}

void DMX512Transmitter::printStatus() const {
    printf("DMX512 Transmitter Status:\n");
    printf("  Initialized: %s\n", _initialized ? "Yes" : "No");
    printf("  GPIO Pin: %u\n", _gpio_pin);
    printf("  Status: ");
    
    switch (_status) {
        case Status::IDLE:
            printf("IDLE\n");
            break;
        case Status::TRANSMITTING_BREAK:
            printf("TRANSMITTING_BREAK\n");
            break;
        case Status::TRANSMITTING_MAB:
            printf("TRANSMITTING_MAB\n");
            break;
        case Status::TRANSMITTING_DATA:
            printf("TRANSMITTING_DATA (byte %u/%u)\n", _current_byte_index, DMX_UNIVERSE_SIZE + 1);
            break;
        case Status::ERROR:
            printf("ERROR\n");
            break;
    }
    
    printf("  Continuous Mode: %s\n", _continuous_mode ? "Enabled" : "Disabled");
    printf("  Frames Transmitted: %lu\n", _frame_count);
    printf("  Errors: %lu\n", _error_count);
    printf("  Start Code: 0x%02X\n", _dmx_frame[0]);
}

void DMX512Transmitter::printFrame(uint16_t start_channel, uint16_t count) const {
    printf("DMX Frame (channels %u-%u):\n", start_channel, start_channel + count - 1);
    
    for (uint16_t i = 0; i < count; i++) {
        uint16_t channel = start_channel + i;
        if (channel <= DMX_UNIVERSE_SIZE) {
            printf("  Ch%03u: %3u (0x%02X)\n", channel, _dmx_frame[channel], _dmx_frame[channel]);
        }
    }
}