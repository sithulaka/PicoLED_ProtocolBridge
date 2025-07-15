/*
 * Copyright (c) 2021 Jostein Løwer 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Description: 
 * LED Pattern to DMX Converter - Creates LED patterns using PicoLED class
 * and converts them to DMX output for external LED controllers
 */

#include <stdio.h>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "DmxOutput.h"
#include "PicoLED.h"
#include "config.h"

// Declare an instance of the DMX Output
DmxOutput dmx;

// Create a universe that we want to send.
// The universe must be maximum 512 bytes + 1 byte of start code
#define UNIVERSE_LENGTH 512
uint8_t universe[UNIVERSE_LENGTH + 1];

// PicoLED instance for pattern generation
PicoLED* led;

void setup_led_pattern_generator() {
    // Initialize PicoLED for pattern generation (no actual WS2812 output needed)
    // We're just using it to manage the LED array and convert to DMX
    led = new PicoLED(pio0, 0, NUM_PIXELS, GRID_WIDTH);
    printf("LED Pattern Generator initialized: %d pixels, %dx%d grid\n", 
           NUM_PIXELS, GRID_WIDTH, GRID_HEIGHT);
}

void play_custom_pattern() {
    printf("Starting custom LED pattern generation...\n");
    
    // Reset all LEDs to off
    led->reset_all_color();
    
    // Pattern sequence as specified - using set_XY (no WS2812 output)
    led->set_XY(8, 1, 0, 0, 100);
    led->set_XY(7, 1, 0, 0, 100);
    led->set_XY(6, 1, 0, 0, 100);
    led->set_XY(6, 2, 0, 0, 100);
    led->set_XY(6, 3, 0, 0, 100);

    led->set_XY(8, 5, 0, 0, 100);
    led->set_XY(8, 6, 0, 0, 100);
    led->set_XY(8, 7, 0, 0, 100);
    led->set_XY(6, 5, 0, 0, 100);
    led->set_XY(6, 6, 0, 0, 100);
    led->set_XY(6, 7, 0, 0, 100);
    led->set_XY(7, 6, 0, 0, 100);

    led->set_XY(4, 1, 0, 0, 100);
    led->set_XY(3, 1, 0, 0, 100);
    led->set_XY(2, 1, 0, 0, 100);
    led->set_XY(2, 2, 0, 0, 100);
    led->set_XY(2, 3, 0, 0, 100);
    led->set_XY(3, 3, 0, 0, 100);
    led->set_XY(4, 3, 0, 0, 100);

    led->set_XY(2, 4, 0, 0, 100);
    led->set_XY(3, 4, 0, 0, 100);
    led->set_XY(4, 4, 0, 0, 100);
    led->set_XY(4, 5, 0, 0, 100);
    led->set_XY(4, 6, 0, 0, 100);
    led->set_XY(3, 6, 0, 0, 100);
    led->set_XY(3, 5, 0, 0, 100);

    led->set_XY(4, 8, 0, 0, 100);
    led->set_XY(4, 7, 0, 0, 100);
    led->set_XY(3, 7, 0, 0, 100);
    led->set_XY(2, 7, 0, 0, 100);
    led->set_XY(2, 8, 0, 0, 100);
    led->set_XY(3, 8, 0, 0, 100);
    
    printf("LED pattern generated in memory (no WS2812 output)\n");
}

void convert_and_send_dmx() {
    // Reset all DMX channels to 0 (including start code)
    memset(universe, 0, sizeof(universe));
    printf("All DMX channels reset to 0\n");
    
    // Convert LED array to DMX universe
    led->GRBArray_to_DmxUniverse_Converter(universe, START_CHANNEL);
    printf("LED array converted to DMX universe\n");
    
    // Send DMX universe
    dmx.write(universe, UNIVERSE_LENGTH);
    
    while (dmx.busy()) {
        /* Wait for transmission to complete */
    }
    
    // Debug: Print some DMX channel values
    printf("DMX Channels: Ch1=%d Ch2=%d Ch3=%d (R,G,B of first LED)\n", 
           universe[1], universe[2], universe[3]);
}

int main()
{
    stdio_init_all();
    printf("LED Pattern to DMX Converter starting...\n");
    
    // Start the DMX Output on GPIO-pin 0
    dmx.begin(DMX_OUT_PIN);
    printf("DMX Output initialized on GPIO %d\n", DMX_OUT_PIN);
    
    // Setup LED pattern generator
    setup_led_pattern_generator();
    
    while (true)
    {
        printf("\n=== Starting new pattern cycle ===\n");
        
        // Play the custom LED pattern
        play_custom_pattern();
        
        // Convert current LED state to DMX and send
        convert_and_send_dmx();
        
        // Debug: Print LED array state
        led->debug_print_led_array();
        
        // Wait before repeating
        printf("Pattern cycle complete. Waiting 3 seconds before repeat...\n");
        sleep_ms(3000);
    }
    
    return 0;
}