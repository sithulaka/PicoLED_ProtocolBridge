#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "../config/picoled_config.h"

/**
 * @brief WS2812 LED Driver using PIO
 * 
 * High-performance driver for WS2812 LED strips and panels
 * Supports DMA transfers for smooth, non-blocking updates
 */
class WS2812Driver {
public:
    enum class ColorFormat {
        RGB,    // Red, Green, Blue
        GRB,    // Green, Red, Blue (WS2812 native)
        RGBW    // Red, Green, Blue, White (for SK6812)
    };

    enum class Status {
        IDLE,
        UPDATING,
        ERROR
    };

    struct Config {
        PIO pio_instance;
        uint pio_sm;
        uint gpio_pin;
        uint num_pixels;
        ColorFormat format;
        bool use_dma;
    };

private:
    // Hardware configuration
    Config _config;
    uint _pio_program_offset;
    
    // Pixel data buffer
    uint32_t* _pixel_buffer;
    volatile Status _status;
    bool _initialized;
    
    // DMA configuration
    int _dma_channel;
    bool _dma_available;
    
    // Statistics
    uint32_t _update_count;
    uint32_t _error_count;
    
    // PIO program for WS2812
    static const uint16_t ws2812_program_instructions[];
    static const struct pio_program ws2812_program;
    
    // Internal methods
    bool init_pio();
    bool init_dma();
    void cleanup_pio();
    void cleanup_dma();
    uint32_t convert_color(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
    void dma_complete_handler();
    static void dma_irq_handler();
    
    // Static instance for DMA interrupt
    static WS2812Driver* _instance;

public:
    /**
     * @brief Constructor
     * @param config Driver configuration
     */
    WS2812Driver(const Config& config);

    /**
     * @brief Destructor
     */
    ~WS2812Driver();

    /**
     * @brief Initialize the WS2812 driver
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Shutdown the driver
     */
    void end();

    /**
     * @brief Set color for specific pixel
     * @param index Pixel index (0-based)
     * @param r Red value (0-255)
     * @param g Green value (0-255)
     * @param b Blue value (0-255)
     * @param w White value (0-255, only for RGBW format)
     * @return true if successful
     */
    bool setPixelColor(uint index, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);

    /**
     * @brief Get color of specific pixel
     * @param index Pixel index
     * @param r Output red value
     * @param g Output green value
     * @param b Output blue value
     * @param w Output white value
     * @return true if successful
     */
    bool getPixelColor(uint index, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w);

    /**
     * @brief Set all pixels to the same color
     * @param r Red value (0-255)
     * @param g Green value (0-255)
     * @param b Blue value (0-255)
     * @param w White value (0-255, only for RGBW format)
     */
    void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);

    /**
     * @brief Clear all pixels (set to black)
     */
    void clear();

    /**
     * @brief Update LED strip/panel with current buffer
     * @param blocking If true, wait for update to complete
     * @return true if update started successfully
     */
    bool update(bool blocking = false);

    /**
     * @brief Check if update is in progress
     */
    bool isBusy() const { return _status == Status::UPDATING; }

    /**
     * @brief Wait for current update to complete
     * @param timeout_ms Maximum time to wait (0 = infinite)
     * @return true if completed within timeout
     */
    bool waitForCompletion(uint32_t timeout_ms = 0);

    /**
     * @brief Set pixel buffer from raw data
     * @param data Source data (format depends on ColorFormat)
     * @param length Number of pixels to copy
     * @param start_index Starting pixel index
     */
    bool setPixelData(const uint8_t* data, uint length, uint start_index = 0);

    /**
     * @brief Get direct access to pixel buffer
     * @return Pointer to pixel buffer (uint32_t per pixel)
     */
    uint32_t* getPixelBuffer() { return _pixel_buffer; }

    /**
     * @brief Get number of pixels
     */
    uint getPixelCount() const { return _config.num_pixels; }

    /**
     * @brief Get color format
     */
    ColorFormat getColorFormat() const { return _config.format; }

    /**
     * @brief Check if driver is initialized
     */
    bool isInitialized() const { return _initialized; }

    /**
     * @brief Get current status
     */
    Status getStatus() const { return _status; }

    /**
     * @brief Get driver configuration
     */
    const Config& getConfig() const { return _config; }

    /**
     * @brief Get statistics
     * @param update_count Total updates performed
     * @param error_count Total errors encountered
     */
    void getStatistics(uint32_t& update_count, uint32_t& error_count) const;

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

    /**
     * @brief Convert RGB values to native format
     * @param r Red value
     * @param g Green value  
     * @param b Blue value
     * @param w White value (for RGBW)
     * @return 32-bit color value in native format
     */
    uint32_t colorToNative(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);

    /**
     * @brief Convert native format to RGB values
     * @param color 32-bit native color value
     * @param r Output red value
     * @param g Output green value
     * @param b Output blue value
     * @param w Output white value
     */
    void nativeToColor(uint32_t color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w);

    /**
     * @brief Set brightness for all pixels (0-255)
     * @param brightness Global brightness multiplier
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief Apply gamma correction to pixel buffer
     * @param gamma Gamma value (typically 2.2)
     */
    void applyGammaCorrection(float gamma = 2.2f);

    // Debug methods
    void printStatus() const;
    void printPixelData(uint start_index = 0, uint count = 16) const;
    
    // Grid/matrix helper methods (for LED panels arranged in grids)
    /**
     * @brief Set pixel color using X,Y coordinates
     * @param x X coordinate
     * @param y Y coordinate  
     * @param r Red value
     * @param g Green value
     * @param b Blue value
     * @param w White value
     * @param grid_width Width of the LED grid
     * @return true if successful
     */
    bool setPixelColorXY(uint x, uint y, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0, uint grid_width = DEFAULT_GRID_WIDTH);

    /**
     * @brief Convert X,Y coordinates to linear index
     * @param x X coordinate
     * @param y Y coordinate
     * @param grid_width Width of the LED grid
     * @return Linear pixel index
     */
    static uint xyToIndex(uint x, uint y, uint grid_width) { return y * grid_width + x; }
};