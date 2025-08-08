#include "../include/PicoLED.h"
#include "pico/stdlib.h"
#include <cstdio>

/**
 * @brief Basic usage example for PicoLED Protocol Bridge
 * 
 * This example demonstrates:
 * - Setting up PicoLED with all three protocols
 * - Controlling WS2812 LED panel
 * - Sending DMX512 data (exactly 512 channels)
 * - Sending RS485 serial data (simplex)
 */

int main() {
    // Initialize stdio for debug output
    stdio_init_all();

    // Configure PicoLED pin assignments
    PicoLED::PinConfig pins = {
        .led_panel_pin = DEFAULT_LED_PIN,        // Pin 2 for WS2812 data
        .dmx512_pin = DEFAULT_DMX_PIN,           // Pin 4 for DMX512 output to RS485
        .rs485_data_pin = DEFAULT_RS485_DATA_PIN, // Pin 8 for RS485 data
        .rs485_enable_pin = DEFAULT_RS485_ENABLE_PIN // Pin 9 for RS485 direction control
    };

    // Configure LED panel settings
    PicoLED::LEDConfig led_config = {
        .num_pixels = DEFAULT_LED_COUNT,         // 256 LEDs
        .grid_width = DEFAULT_GRID_WIDTH,        // 16x16 grid
        .grid_height = DEFAULT_GRID_HEIGHT,      // 16x16 grid
        .pio_instance = WS2812_PIO,              // Use PIO0
        .pio_sm = WS2812_SM                      // Use state machine 0
    };

    // Create PicoLED instance
    PicoLED picoled(pins, led_config);

    // Initialize all protocols
    if (!picoled.begin()) {
        printf("ERROR: Failed to initialize PicoLED!\n");
        return -1;
    }

    printf("PicoLED Protocol Bridge initialized successfully!\n");
    picoled.printStatus();

    // Main loop demonstration
    uint32_t loop_count = 0;
    absolute_time_t last_update = get_absolute_time();

    while (true) {
        absolute_time_t now = get_absolute_time();
        uint32_t elapsed_ms = absolute_time_diff_us(last_update, now) / 1000;

        if (elapsed_ms >= UPDATE_INTERVAL_MS) {
            last_update = now;
            
            // Example 1: Create rainbow effect on LED panel
            for (uint y = 0; y < led_config.grid_height; y++) {
                for (uint x = 0; x < led_config.grid_width; x++) {
                    // Create rainbow based on position and time
                    uint8_t hue = (x * 16 + y * 16 + loop_count) % 256;
                    uint8_t r = (hue < 85) ? (255 - hue * 3) : (hue < 170) ? 0 : ((hue - 170) * 3);
                    uint8_t g = (hue < 85) ? (hue * 3) : (hue < 170) ? (255 - (hue - 85) * 3) : 0;
                    uint8_t b = (hue < 85) ? 0 : (hue < 170) ? ((hue - 85) * 3) : (255 - (hue - 170) * 3);
                    
                    picoled.setLEDColorXY(x, y, r, g, b);
                }
            }

            // Update LED panel
            picoled.updateLEDPanel();

            // Example 2: Convert LED data to DMX and transmit
            picoled.ledsToDMX(1);  // Start from DMX channel 1
            picoled.transmitDMX();

            // Example 3: Send status message via RS485
            if (loop_count % 100 == 0) {  // Every ~1.6 seconds
                char status_msg[64];
                snprintf(status_msg, sizeof(status_msg), "PicoLED Status: Loop %lu\n", loop_count);
                picoled.sendRS485String(status_msg);
            }

            loop_count++;

            // Print periodic status
            if (loop_count % 1000 == 0) {
                printf("Loop count: %lu\n", loop_count);
                picoled.printStatus();
            }
        }

        // Small delay to prevent busy waiting
        sleep_ms(1);
    }

    return 0;
}