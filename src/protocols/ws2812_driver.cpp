#include "ws2812_driver.h"
#include <cstring>
#include <cstdio>
#include <cmath>

// Static instance for DMA interrupt handling
WS2812Driver* WS2812Driver::_instance = nullptr;

// WS2812 PIO program (optimized for RP2040)
const uint16_t WS2812Driver::ws2812_program_instructions[] = {
    0x6221, // out    x, 1            side 0 [2] 
    0x1123, // jmp    !x, 3           side 1 [1] 
    0x1400, // jmp    0               side 1 [4] 
    0xa442, // nop                    side 0 [4] 
};

const struct pio_program WS2812Driver::ws2812_program = {
    .instructions = ws2812_program_instructions,
    .length = 4,
    .origin = -1
};

WS2812Driver::WS2812Driver(const Config& config) 
    : _config(config),
      _pio_program_offset(0),
      _pixel_buffer(nullptr),
      _status(Status::IDLE),
      _initialized(false),
      _dma_channel(-1),
      _dma_available(false),
      _update_count(0),
      _error_count(0) {
    
    _instance = this;
}

WS2812Driver::~WS2812Driver() {
    if (_initialized) {
        end();
    }
    
    if (_instance == this) {
        _instance = nullptr;
    }
}

bool WS2812Driver::begin() {
    if (_initialized) {
        return true;
    }

    // Validate configuration
    if (_config.num_pixels == 0 || _config.num_pixels > MAX_LED_COUNT) {
        return false;
    }

    // Allocate pixel buffer
    size_t buffer_size = _config.num_pixels * sizeof(uint32_t);
    _pixel_buffer = (uint32_t*)malloc(buffer_size);
    if (_pixel_buffer == nullptr) {
        return false;
    }

    // Initialize buffer to all black
    memset(_pixel_buffer, 0, buffer_size);

    // Initialize PIO
    if (!init_pio()) {
        free(_pixel_buffer);
        _pixel_buffer = nullptr;
        return false;
    }

    // Initialize DMA if requested and available
    if (_config.use_dma) {
        _dma_available = init_dma();
    }

    _initialized = true;
    _status = Status::IDLE;
    
    return true;
}

void WS2812Driver::end() {
    if (!_initialized) {
        return;
    }

    // Wait for any ongoing updates
    waitForCompletion(1000);

    // Cleanup resources
    cleanup_dma();
    cleanup_pio();

    if (_pixel_buffer != nullptr) {
        free(_pixel_buffer);
        _pixel_buffer = nullptr;
    }

    _initialized = false;
    _status = Status::IDLE;
}

bool WS2812Driver::init_pio() {
    // Load PIO program
    _pio_program_offset = pio_add_program(_config.pio_instance, &ws2812_program);
    
    // Get state machine
    uint sm = _config.pio_sm;
    if (!pio_sm_is_claimed(_config.pio_instance, sm)) {
        pio_sm_claim(_config.pio_instance, sm);
    }

    // Configure state machine
    pio_sm_config config = pio_get_default_sm_config();
    sm_config_set_wrap(&config, _pio_program_offset, _pio_program_offset + ws2812_program.length - 1);
    sm_config_set_sideset_pins(&config, _config.gpio_pin);
    
    // Set output shift direction and auto-pull
    sm_config_set_out_shift(&config, false, true, 24);  // 24-bit color data
    
    // Set clock divider for WS2812 timing (800 kHz)
    float div = (float)clock_get_hz(clk_sys) / (800000 * 4);  // 4 cycles per bit
    sm_config_set_clkdiv(&config, div);

    // Configure GPIO
    pio_gpio_init(_config.pio_instance, _config.gpio_pin);
    pio_sm_set_consecutive_pindirs(_config.pio_instance, sm, _config.gpio_pin, 1, true);

    // Initialize and start state machine
    pio_sm_init(_config.pio_instance, sm, _pio_program_offset, &config);
    pio_sm_set_enabled(_config.pio_instance, sm, true);

    return true;
}

void WS2812Driver::cleanup_pio() {
    uint sm = _config.pio_sm;
    
    // Stop state machine
    pio_sm_set_enabled(_config.pio_instance, sm, false);
    
    // Unclaim state machine
    pio_sm_unclaim(_config.pio_instance, sm);
    
    // Remove program
    pio_remove_program(_config.pio_instance, &ws2812_program, _pio_program_offset);
}

bool WS2812Driver::init_dma() {
    // Try to claim a DMA channel
    _dma_channel = dma_claim_unused_channel(false);
    if (_dma_channel < 0) {
        return false;  // No DMA channels available
    }

    // Configure DMA channel
    dma_channel_config config = dma_channel_get_default_config(_dma_channel);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_32);
    channel_config_set_read_increment(&config, true);
    channel_config_set_write_increment(&config, false);
    channel_config_set_dreq(&config, pio_get_dreq(_config.pio_instance, _config.pio_sm, true));

    // Configure interrupt
    dma_channel_set_irq0_enabled(_dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_configure(_dma_channel, &config, 
                         &_config.pio_instance->txf[_config.pio_sm],
                         _pixel_buffer,
                         _config.num_pixels,
                         false);  // Don't start yet

    return true;
}

void WS2812Driver::cleanup_dma() {
    if (_dma_channel >= 0) {
        dma_channel_abort(_dma_channel);
        dma_channel_set_irq0_enabled(_dma_channel, false);
        dma_channel_unclaim(_dma_channel);
        _dma_channel = -1;
    }
    _dma_available = false;
}

bool WS2812Driver::setPixelColor(uint index, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    if (!_initialized || index >= _config.num_pixels) {
        return false;
    }

    _pixel_buffer[index] = convert_color(r, g, b, w);
    return true;
}

bool WS2812Driver::getPixelColor(uint index, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w) {
    if (!_initialized || index >= _config.num_pixels) {
        return false;
    }

    nativeToColor(_pixel_buffer[index], r, g, b, w);
    return true;
}

void WS2812Driver::fill(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    if (!_initialized) {
        return;
    }

    uint32_t color = convert_color(r, g, b, w);
    for (uint i = 0; i < _config.num_pixels; i++) {
        _pixel_buffer[i] = color;
    }
}

void WS2812Driver::clear() {
    if (!_initialized || _pixel_buffer == nullptr) {
        return;
    }

    memset(_pixel_buffer, 0, _config.num_pixels * sizeof(uint32_t));
}

bool WS2812Driver::update(bool blocking) {
    if (!_initialized || _status == Status::UPDATING) {
        return false;
    }

    _status = Status::UPDATING;

    if (_dma_available) {
        // Use DMA for non-blocking transfer
        dma_channel_set_read_addr(_dma_channel, _pixel_buffer, true);
        
        if (blocking) {
            waitForCompletion();
        }
    } else {
        // Use PIO directly (blocking)
        for (uint i = 0; i < _config.num_pixels; i++) {
            pio_sm_put_blocking(_config.pio_instance, _config.pio_sm, _pixel_buffer[i] << 8);
        }
        
        // Wait for WS2812 reset time
        busy_wait_us(WS2812_RESET_TIME_US);
        
        _status = Status::IDLE;
        _update_count++;
    }

    return true;
}

bool WS2812Driver::waitForCompletion(uint32_t timeout_ms) {
    absolute_time_t start_time = get_absolute_time();

    while (_status == Status::UPDATING) {
        if (timeout_ms > 0) {
            if (absolute_time_diff_us(start_time, get_absolute_time()) > (timeout_ms * 1000)) {
                return false;  // Timeout
            }
        }
        tight_loop_contents();
    }

    return true;
}

bool WS2812Driver::setPixelData(const uint8_t* data, uint length, uint start_index) {
    if (!_initialized || data == nullptr || start_index >= _config.num_pixels) {
        return false;
    }

    uint max_pixels = _config.num_pixels - start_index;
    if (length > max_pixels) {
        length = max_pixels;
    }

    // Convert based on color format
    uint bytes_per_pixel = (_config.format == ColorFormat::RGBW) ? 4 : 3;
    
    for (uint i = 0; i < length; i++) {
        uint data_offset = i * bytes_per_pixel;
        uint8_t r, g, b, w = 0;
        
        switch (_config.format) {
            case ColorFormat::RGB:
                r = data[data_offset];
                g = data[data_offset + 1];
                b = data[data_offset + 2];
                break;
            case ColorFormat::GRB:
                g = data[data_offset];
                r = data[data_offset + 1];
                b = data[data_offset + 2];
                break;
            case ColorFormat::RGBW:
                r = data[data_offset];
                g = data[data_offset + 1];
                b = data[data_offset + 2];
                w = data[data_offset + 3];
                break;
        }
        
        _pixel_buffer[start_index + i] = convert_color(r, g, b, w);
    }

    return true;
}

uint32_t WS2812Driver::convert_color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    switch (_config.format) {
        case ColorFormat::RGB:
            return (r << 16) | (g << 8) | b;
        case ColorFormat::GRB:
            return (g << 16) | (r << 8) | b;  // WS2812 native format
        case ColorFormat::RGBW:
            return (w << 24) | (r << 16) | (g << 8) | b;
        default:
            return 0;
    }
}

uint32_t WS2812Driver::colorToNative(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return convert_color(r, g, b, w);
}

void WS2812Driver::nativeToColor(uint32_t color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w) {
    switch (_config.format) {
        case ColorFormat::RGB:
            r = (color >> 16) & 0xFF;
            g = (color >> 8) & 0xFF;
            b = color & 0xFF;
            w = 0;
            break;
        case ColorFormat::GRB:
            g = (color >> 16) & 0xFF;
            r = (color >> 8) & 0xFF;
            b = color & 0xFF;
            w = 0;
            break;
        case ColorFormat::RGBW:
            w = (color >> 24) & 0xFF;
            r = (color >> 16) & 0xFF;
            g = (color >> 8) & 0xFF;
            b = color & 0xFF;
            break;
    }
}

void WS2812Driver::setBrightness(uint8_t brightness) {
    if (!_initialized) {
        return;
    }

    for (uint i = 0; i < _config.num_pixels; i++) {
        uint8_t r, g, b, w;
        nativeToColor(_pixel_buffer[i], r, g, b, w);
        
        // Apply brightness scaling
        r = (r * brightness) / 255;
        g = (g * brightness) / 255;
        b = (b * brightness) / 255;
        w = (w * brightness) / 255;
        
        _pixel_buffer[i] = convert_color(r, g, b, w);
    }
}

void WS2812Driver::applyGammaCorrection(float gamma) {
    if (!_initialized) {
        return;
    }

    // Create gamma correction lookup table
    uint8_t gamma_table[256];
    for (int i = 0; i < 256; i++) {
        gamma_table[i] = (uint8_t)(pow(i / 255.0, gamma) * 255.0 + 0.5);
    }

    // Apply gamma correction to all pixels
    for (uint i = 0; i < _config.num_pixels; i++) {
        uint8_t r, g, b, w;
        nativeToColor(_pixel_buffer[i], r, g, b, w);
        
        r = gamma_table[r];
        g = gamma_table[g];
        b = gamma_table[b];
        w = gamma_table[w];
        
        _pixel_buffer[i] = convert_color(r, g, b, w);
    }
}

bool WS2812Driver::setPixelColorXY(uint x, uint y, uint8_t r, uint8_t g, uint8_t b, uint8_t w, uint grid_width) {
    uint index = xyToIndex(x, y, grid_width);
    return setPixelColor(index, r, g, b, w);
}

void WS2812Driver::getStatistics(uint32_t& update_count, uint32_t& error_count) const {
    update_count = _update_count;
    error_count = _error_count;
}

void WS2812Driver::resetStatistics() {
    _update_count = 0;
    _error_count = 0;
}

void WS2812Driver::dma_irq_handler() {
    if (_instance != nullptr) {
        _instance->dma_complete_handler();
    }
}

void WS2812Driver::dma_complete_handler() {
    if (dma_channel_get_irq0_status(_dma_channel)) {
        dma_channel_acknowledge_irq0(_dma_channel);
        
        // Wait for WS2812 reset time
        busy_wait_us(WS2812_RESET_TIME_US);
        
        _status = Status::IDLE;
        _update_count++;
    }
}

void WS2812Driver::printStatus() const {
    printf("WS2812 Driver Status:\n");
    printf("  Initialized: %s\n", _initialized ? "Yes" : "No");
    printf("  GPIO Pin: %u\n", _config.gpio_pin);
    printf("  Pixels: %u\n", _config.num_pixels);
    printf("  Format: ");
    
    switch (_config.format) {
        case ColorFormat::RGB:
            printf("RGB\n");
            break;
        case ColorFormat::GRB:
            printf("GRB\n");
            break;
        case ColorFormat::RGBW:
            printf("RGBW\n");
            break;
    }
    
    printf("  DMA Enabled: %s\n", _dma_available ? "Yes" : "No");
    printf("  Status: ");
    
    switch (_status) {
        case Status::IDLE:
            printf("IDLE\n");
            break;
        case Status::UPDATING:
            printf("UPDATING\n");
            break;
        case Status::ERROR:
            printf("ERROR\n");
            break;
    }
    
    printf("  Updates: %lu\n", _update_count);
    printf("  Errors: %lu\n", _error_count);
}

void WS2812Driver::printPixelData(uint start_index, uint count) const {
    printf("Pixel Data (index %u-%u):\n", start_index, start_index + count - 1);
    
    for (uint i = 0; i < count && (start_index + i) < _config.num_pixels; i++) {
        uint index = start_index + i;
        uint8_t r, g, b, w;
        nativeToColor(_pixel_buffer[index], r, g, b, w);
        
        if (_config.format == ColorFormat::RGBW) {
            printf("  Pixel[%3u]: R=%3u G=%3u B=%3u W=%3u (0x%08lX)\n", 
                   index, r, g, b, w, _pixel_buffer[index]);
        } else {
            printf("  Pixel[%3u]: R=%3u G=%3u B=%3u (0x%08lX)\n", 
                   index, r, g, b, _pixel_buffer[index]);
        }
    }
}