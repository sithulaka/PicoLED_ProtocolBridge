/*
 * DMX Sender for Raspberry Pi Pico RP2040
 * 
 * This program sets LED colors using PicoLED library, converts the LED array
 * to DMX universe format, and sends it via DMX output using Pico-DMX library.
 * 
 * The LED pattern creates animated text/shapes on an 8x8 LED matrix.
 */

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "PicoLED.h"
#include "config.h"
#include "DmxOutput.h"
#include "ws2812.pio.h"
#include <stdio.h>

// DMX and LED instances
DmxOutput dmxOutput;
PicoLED led(pio0, 0, NUM_PIXELS, GRID_WIDTH);

// DMX universe buffer (512 channels + start code)
uint8_t dmx_universe[513];

// Function to send DMX universe using PicoLED converter
void sendDmxUniverse() {
    printf("[DMX-SENDER] Converting LED array to DMX universe using PicoLED...\n");
    
    // Use PicoLED's built-in converter function
    led.GRBArray_to_DmxUniverse_Converter(dmx_universe, START_CHANNEL);
    
    printf("[DMX-SENDER] Sending DMX universe...\n");
    dmxOutput.write(dmx_universe, 513); // Full universe (512 channels + start code)
    
    // Wait for transmission to complete
    while (dmxOutput.busy()) {
        tight_loop_contents();
    }
    
    printf("[DMX-SENDER] DMX universe sent successfully\n");
}

// Function to create the LED animation pattern
void createLedPattern() {
    printf("[DMX-SENDER] Starting LED pattern animation...\n");
    
    // Reset all LEDs
    printf("[DMX-SENDER] Resetting all LEDs...\n");
    led.reset_all_color();
    
    // Draw all characters without sending DMX (build complete pattern)
    printf("[DMX-SENDER] Drawing complete character pattern...\n");
    
    // First character/shape
    led.set_XY(8, 1, 0, 0, 100);
    led.set_XY(7, 1, 0, 0, 100);
    led.set_XY(6, 1, 0, 0, 100);
    led.set_XY(6, 2, 0, 0, 100);
    led.set_XY(6, 3, 0, 0, 100);

    // Second character/shape  
    led.set_XY(8, 5, 0, 0, 100);
    led.set_XY(8, 6, 0, 0, 100);
    led.set_XY(8, 7, 0, 0, 100);
    led.set_XY(6, 5, 0, 0, 100);
    led.set_XY(6, 6, 0, 0, 100);
    led.set_XY(6, 7, 0, 0, 100);
    led.set_XY(7, 6, 0, 0, 100);

    // Third character/shape
    led.set_XY(4, 1, 0, 0, 100);
    led.set_XY(3, 1, 0, 0, 100);
    led.set_XY(2, 1, 0, 0, 100);
    led.set_XY(2, 2, 0, 0, 100);
    led.set_XY(2, 3, 0, 0, 100);
    led.set_XY(3, 3, 0, 0, 100);
    led.set_XY(4, 3, 0, 0, 100);

    // Fourth character/shape
    led.set_XY(2, 4, 0, 0, 100);
    led.set_XY(3, 4, 0, 0, 100);
    led.set_XY(4, 4, 0, 0, 100);
    led.set_XY(4, 5, 0, 0, 100);
    led.set_XY(4, 6, 0, 0, 100);
    led.set_XY(3, 6, 0, 0, 100);
    led.set_XY(3, 5, 0, 0, 100);

    // Fifth character/shape
    led.set_XY(4, 8, 0, 0, 100);
    led.set_XY(4, 7, 0, 0, 100);
    led.set_XY(3, 7, 0, 0, 100);
    led.set_XY(2, 7, 0, 0, 100);
    led.set_XY(2, 8, 0, 0, 100);
    led.set_XY(3, 8, 0, 0, 100);
      // Now push the complete pattern to physical LEDs and send as one DMX universe
    printf("[DMX-SENDER] Pushing complete pattern to LEDs...\n");
    
    // Debug: Print LED array state
    led.debug_print_led_array();
    
    printf("[DMX-SENDER] Sending complete character pattern as one DMX universe...\n");
    sendDmxUniverse();
    
    printf("[DMX-SENDER] Complete LED pattern sent in one universe!\n");
    
    // Wait before next cycle
    sleep_ms(2000);
}

// Test function to verify the conversion functions work correctly
void testConversionFunctions() {
    printf("\n[TEST] Testing PicoLED conversion functions...\n");
    
    // Test 1: Set some LED colors manually
    printf("[TEST] Setting test pattern: LED 0=Red, LED 1=Green, LED 2=Blue\n");
    led.set_color(1, 255, 0, 0);   // Red
    led.set_color(2, 0, 255, 0);   // Green  
    led.set_color(3, 0, 0, 255);   // Blue
    
    // Test 2: Convert to DMX universe
    printf("[TEST] Converting to DMX universe...\n");
    led.GRBArray_to_DmxUniverse_Converter(dmx_universe, START_CHANNEL);
    
    // Print first few DMX channels to verify
    printf("[TEST] DMX channels 1-9: ");
    for (int i = 1; i <= 9; i++) {
        printf("%d ", dmx_universe[i]);
    }
    printf("\n");
    
    // Test 3: Create a test DMX array and convert back
    printf("[TEST] Testing reverse conversion...\n");
    uint8_t test_dmx[12] = {100, 150, 200,  // LED 0: R=100, G=150, B=200
                           50, 75, 25,      // LED 1: R=50, G=75, B=25  
                           255, 128, 64,    // LED 2: R=255, G=128, B=64
                           0, 0, 0};        // LED 3: OFF
    
    led.reset_all_color(); // Clear LEDs first
    led.DmxArray_to_GRBArray_Converter(test_dmx);
    
    printf("[TEST] After DMX to GRB conversion:\n");
    led.debug_print_led_array();
    
    printf("[TEST] Conversion function tests completed!\n\n");
}

int main() {
    stdio_init_all();
    
    printf("\n=== DMX SENDER STARTING ===\n");
    printf("[DMX-SENDER] Initializing system...\n");
    
    // Initialize PIO for WS2812 LEDs
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, IS_RGBW);
    
    printf("[DMX-SENDER] PicoLED initialized on pin %d\n", WS2812_PIN);
    
    // Initialize DMX output
    DmxOutput::return_code dmx_result = dmxOutput.begin(DMX_IN_PIN, pio1);
    if (dmx_result != DmxOutput::SUCCESS) {
        printf("[DMX-SENDER] ERROR: Failed to initialize DMX output: %d\n", dmx_result);
        return -1;
    }
      printf("[DMX-SENDER] DMX output initialized on pin %d\n", DMX_IN_PIN);
    printf("[DMX-SENDER] DMX Universe: Full 512 channels\n");
    printf("[DMX-SENDER] LED Matrix: %dx%d (%d pixels)\n", GRID_WIDTH, GRID_HEIGHT, NUM_PIXELS);
    printf("[DMX-SENDER] LED data will use channels %d-%d (%d channels total)\n", 
           START_CHANNEL, START_CHANNEL + (NUM_PIXELS * 3) - 1, NUM_PIXELS * 3);
    
    // Initialize DMX universe buffer
    for (int i = 0; i < 513; i++) {
        dmx_universe[i] = 0;
    }
      printf("[DMX-SENDER] System initialization complete!\n");
    
    // Run conversion function tests first
    testConversionFunctions();
    
    // Main loop - continuously run the LED pattern and send DMX
    while (true) {
        printf("\n[DMX-SENDER] Starting new animation cycle...\n");
        createLedPattern();
        
        printf("[DMX-SENDER] Animation cycle complete. Waiting 3 seconds before restart...\n");
        sleep_ms(3000);
    }
    
    return 0;
}