#pragma once

// LED configuration
#define IS_RGBW false       // Set to true for RGBW LEDs
#define NUM_PIXELS 64       // Total number of pixels in the strip/matrix
#define GRID_WIDTH 8        // Width of the LED grid (for matrix layouts)
#define GRID_HEIGHT 8       // Height of the LED grid (for matrix layouts)

// DMX configuration
#define START_CHANNEL 1     // Starting DMX channel
#define NUM_CHANNELS 512    // Number of DMX channels to read

// Hardware pin assignments
#define DMX_IN_PIN 1        // GPIO pin for DMX input
#define WS2812_PIN 16       // GPIO pin for WS2812 LED data
#define WS2812_FREQ 800000  // WS2812 data frequency 