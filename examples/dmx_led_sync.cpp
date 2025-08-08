#include "../include/PicoLED.h"
#include "pico/stdlib.h"
#include <cstdio>

/**
 * @brief DMX-LED Synchronization Example
 * 
 * This example demonstrates:
 * - Synchronizing WS2812 LED panel with DMX512 output
 * - Reading DMX data from external source and displaying on LEDs
 * - Sending LED patterns as DMX512 data
 */

int main() {
    stdio_init_all();

    // Configure pins
    PicoLED::PinConfig pins = {
        .led_panel_pin = 2,
        .dmx512_pin = 4,
        .rs485_data_pin = 8,
        .rs485_enable_pin = 9
    };

    // Configure for 8x8 LED grid (64 LEDs)
    PicoLED::LEDConfig led_config = {
        .num_pixels = 64,
        .grid_width = 8,
        .grid_height = 8,
        .pio_instance = pio0,
        .pio_sm = 0
    };

    PicoLED picoled(pins, led_config);

    if (!picoled.begin()) {
        printf("ERROR: Failed to initialize PicoLED!\n");
        return -1;
    }

    printf("DMX-LED Sync Demo Started!\n");

    // Demo patterns
    const char* pattern_names[] = {
        "Red Sweep",
        "Green Sweep", 
        "Blue Sweep",
        "Rainbow",
        "Checkerboard",
        "All White"
    };

    uint pattern = 0;
    uint32_t loop_count = 0;

    while (true) {
        // Change pattern every 5 seconds (assuming 60 FPS)
        if (loop_count % 300 == 0) {
            pattern = (pattern + 1) % 6;
            printf("Switching to pattern: %s\n", pattern_names[pattern]);
            
            // Send pattern name via RS485
            char msg[64];
            snprintf(msg, sizeof(msg), "PATTERN: %s\n", pattern_names[pattern]);
            picoled.sendRS485String(msg);
        }

        // Generate pattern
        switch (pattern) {
            case 0: // Red Sweep
                {
                    uint sweep_pos = (loop_count / 8) % 8;
                    picoled.clearAllLEDs();
                    for (uint x = 0; x < 8; x++) {
                        uint8_t intensity = (x == sweep_pos) ? 255 : 0;
                        picoled.setLEDColorXY(x, sweep_pos, intensity, 0, 0);
                    }
                }
                break;

            case 1: // Green Sweep
                {
                    uint sweep_pos = (loop_count / 8) % 8;
                    picoled.clearAllLEDs();
                    for (uint y = 0; y < 8; y++) {
                        uint8_t intensity = (y == sweep_pos) ? 255 : 0;
                        picoled.setLEDColorXY(sweep_pos, y, 0, intensity, 0);
                    }
                }
                break;

            case 2: // Blue Sweep
                {
                    uint sweep_pos = (loop_count / 8) % 8;
                    picoled.clearAllLEDs();
                    for (uint x = 0; x < 8; x++) {
                        for (uint y = 0; y < 8; y++) {
                            if ((x + y) % 8 == sweep_pos) {
                                picoled.setLEDColorXY(x, y, 0, 0, 255);
                            }
                        }
                    }
                }
                break;

            case 3: // Rainbow
                {
                    for (uint x = 0; x < 8; x++) {
                        for (uint y = 0; y < 8; y++) {
                            uint8_t hue = (x * 32 + y * 32 + loop_count) % 256;
                            uint8_t r = (hue < 85) ? (255 - hue * 3) : (hue < 170) ? 0 : ((hue - 170) * 3);
                            uint8_t g = (hue < 85) ? (hue * 3) : (hue < 170) ? (255 - (hue - 85) * 3) : 0;
                            uint8_t b = (hue < 85) ? 0 : (hue < 170) ? ((hue - 85) * 3) : (255 - (hue - 170) * 3);
                            picoled.setLEDColorXY(x, y, r, g, b);
                        }
                    }
                }
                break;

            case 4: // Checkerboard
                {
                    bool phase = (loop_count / 30) % 2;
                    for (uint x = 0; x < 8; x++) {
                        for (uint y = 0; y < 8; y++) {
                            bool checker = ((x + y) % 2) ^ phase;
                            uint8_t intensity = checker ? 255 : 0;
                            picoled.setLEDColorXY(x, y, intensity, intensity, intensity);
                        }
                    }
                }
                break;

            case 5: // All White
                {
                    uint8_t intensity = (uint8_t)(128 + 127 * sin(loop_count * 0.1));
                    picoled.setAllLEDs(intensity, intensity, intensity);
                }
                break;
        }

        // Update LED panel
        picoled.updateLEDPanel();

        // Convert LED data to DMX and transmit
        // LEDs use channels 1-192 (64 LEDs * 3 channels each)
        picoled.ledsToDMX(1);

        // Add some moving light data in remaining DMX channels
        // Channels 193-200: Moving light parameters
        picoled.setDMXChannel(193, (uint8_t)(127 + 127 * sin(loop_count * 0.05)));  // Pan
        picoled.setDMXChannel(194, (uint8_t)(127 + 127 * cos(loop_count * 0.05)));  // Tilt
        picoled.setDMXChannel(195, 255);  // Intensity
        picoled.setDMXChannel(196, (loop_count / 2) % 256);  // Color wheel
        picoled.setDMXChannel(197, 0);    // Gobo
        picoled.setDMXChannel(198, 0);    // Prism
        picoled.setDMXChannel(199, 0);    // Focus
        picoled.setDMXChannel(200, 0);    // Reserved

        // Transmit exactly 512 channels of DMX
        picoled.transmitDMX();

        // Status report every 10 seconds
        if (loop_count % 600 == 0) {
            printf("Status at loop %lu:\n", loop_count);
            printf("  Pattern: %s\n", pattern_names[pattern]);
            printf("  DMX Busy: %s\n", picoled.isDMXBusy() ? "Yes" : "No");
            printf("  RS485 Busy: %s\n", picoled.isRS485Busy() ? "Yes" : "No");
        }

        loop_count++;
        sleep_ms(16);  // ~60 FPS
    }

    return 0;
}