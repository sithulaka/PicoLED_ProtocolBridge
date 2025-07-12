/*
 * DMX Receiver for Raspberry Pi Pico RP2040
 * 
 * This program receives DMX packets on one core and processes them on another core.
 * Core 0: Receives DMX universe data and stores it in a shared buffer
 * Core 1: Converts DMX data to PicoLED format and outputs to WS2812 serial LEDs
 * 
 * Uses dual-core architecture for real-time DMX processing and LED updates.
 */

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/mutex.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "PicoLED.h"
#include "config.h"
#include "DmxInput.h"
#include "ws2812.pio.h"
#include <stdio.h>

// Helper function to get current time in milliseconds
static inline uint32_t millis() {
    return to_ms_since_boot(get_absolute_time());
}

// Global instances
DmxInput dmxInput;
PicoLED led(pio0, 0, NUM_PIXELS, GRID_WIDTH);

// Shared data between cores
volatile bool new_dmx_data = false;
volatile bool system_ready = false;
volatile uint32_t dmx_packet_count = 0;
volatile uint32_t led_update_count = 0;

// DMX buffer for receiving data
volatile uint8_t dmx_buffer[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];

// Shared LED color buffer for inter-core communication
volatile uint8_t shared_rgb_buffer[NUM_PIXELS * 3];

// Mutex for thread-safe access to shared data
mutex_t data_mutex;

// DMX input callback function (called when new DMX data arrives)
void dmxInputCallback(DmxInput* instance) {
    if (!mutex_try_enter(&data_mutex, NULL)) {
        return; // Skip if mutex is busy
    }
    
    // Copy DMX data to shared RGB buffer
    for (uint i = 0; i < NUM_CHANNELS && i < (NUM_PIXELS * 3); i++) {
        shared_rgb_buffer[i] = dmx_buffer[i + 1]; // Skip start code
    }
    
    new_dmx_data = true;
    uint32_t temp_count = dmx_packet_count + 1;
    dmx_packet_count = temp_count;
    
    mutex_exit(&data_mutex);
    
    printf("[CORE0-DMX] Packet #%lu received, %d channels\n", dmx_packet_count, NUM_CHANNELS);
}

// Core 0 function: DMX reception
void core0_dmx_receiver() {
    printf("[CORE0-DMX] Starting DMX receiver on core 0...\n");
    
    // Initialize DMX input with async reading and callback
    DmxInput::return_code dmx_result = dmxInput.begin(DMX_IN_PIN, START_CHANNEL, NUM_CHANNELS, pio1, false);
    if (dmx_result != DmxInput::SUCCESS) {
        printf("[CORE0-DMX] ERROR: Failed to initialize DMX input: %d\n", dmx_result);
        return;
    }
    
    printf("[CORE0-DMX] DMX input initialized on pin %d\n", DMX_IN_PIN);
    printf("[CORE0-DMX] Listening for DMX on channels %d-%d\n", START_CHANNEL, START_CHANNEL + NUM_CHANNELS - 1);
    
    // Start async DMX reading with callback
    dmxInput.read_async(dmx_buffer, dmxInputCallback);
    
    printf("[CORE0-DMX] DMX receiver ready and listening...\n");
    system_ready = true;
    
    uint32_t last_packet_count = 0;
    uint32_t no_data_counter = 0;
    
    // Core 0 main loop - monitor DMX reception
    while (true) {
        sleep_ms(100); // Check every 100ms
        
        // Check for DMX timeout (no data received)
        if (millis() > dmxInput.latest_packet_timestamp() + 1000) {
            no_data_counter++;
            if (no_data_counter > 10) { // Print warning every ~1 second
                printf("[CORE0-DMX] WARNING: No DMX data for %lu ms\n", 
                       millis() - dmxInput.latest_packet_timestamp());
                no_data_counter = 0;
            }
        } else {
            no_data_counter = 0;
        }
        
        // Log DMX reception rate every 5 seconds
        if (dmx_packet_count != last_packet_count) {
            if ((dmx_packet_count % 50) == 0) { // Every 50 packets
                printf("[CORE0-DMX] Total packets received: %lu (Rate: ~%lu pps)\n", 
                       dmx_packet_count, dmx_packet_count * 1000 / millis());
            }
            last_packet_count = dmx_packet_count;
        }
    }
}

// Core 1 function: LED processing and output
void core1_led_processor() {
    printf("[CORE1-LED] Starting LED processor on core 1...\n");
    
    // Wait for system to be ready
    while (!system_ready) {
        sleep_ms(10);
    }
    
    printf("[CORE1-LED] LED processor ready!\n");
    
    uint8_t local_rgb_buffer[NUM_PIXELS * 3];
    
    // Core 1 main loop - process DMX data and update LEDs
    while (true) {
        if (new_dmx_data) {
            // Copy shared data to local buffer with mutex protection
            mutex_enter_blocking(&data_mutex);
            for (uint i = 0; i < NUM_PIXELS * 3; i++) {
                local_rgb_buffer[i] = shared_rgb_buffer[i];
            }
            new_dmx_data = false;
            mutex_exit(&data_mutex);
              // Convert DMX RGB data to PicoLED format and update LEDs
            printf("[CORE1-LED] Processing DMX data for %d LEDs...\n", NUM_PIXELS);
            
            // Convert RGB array to LED array
            led.DmxArray_to_GRBArray_Converter(local_rgb_buffer);
            
            // Debug: Print first few LED values to verify conversion
            if ((led_update_count % 10) == 0) { // Print debug info every 10 updates
                printf("[CORE1-LED] Debug: First 3 DMX RGB values: R=%d G=%d B=%d\n", 
                       local_rgb_buffer[0], local_rgb_buffer[1], local_rgb_buffer[2]);
                led.debug_print_led_array();
            }
            
            // Push to physical LEDs
            led.push_array();
            
            uint32_t temp_count = led_update_count + 1;
            led_update_count = temp_count;
            
            printf("[CORE1-LED] LED update #%lu completed\n", led_update_count);
            
            // Log update rate periodically
            if ((led_update_count % 20) == 0) { // Every 20 updates
                printf("[CORE1-LED] Total LED updates: %lu (Rate: ~%lu ups)\n", 
                       led_update_count, led_update_count * 1000 / millis());
            }
        }
        
        sleep_ms(20); // Check for new data every 20ms (50 FPS max)
    }
}

int main() {
    stdio_init_all();
    
    printf("\n=== DMX RECEIVER STARTING ===\n");
    printf("[MAIN] Initializing dual-core DMX receiver...\n");
    
    // Initialize mutex for thread-safe communication
    mutex_init(&data_mutex);
    
    // Initialize PIO for WS2812 LEDs
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, IS_RGBW);
    
    printf("[MAIN] PicoLED initialized on pin %d\n", WS2812_PIN);
    printf("[MAIN] LED Matrix: %dx%d (%d pixels)\n", GRID_WIDTH, GRID_HEIGHT, NUM_PIXELS);
    printf("[MAIN] DMX Channels: %d (starting from channel %d)\n", NUM_CHANNELS, START_CHANNEL);
    
    // Initialize LED array to off state
    led.reset_all_color();
    led.push_array();
    
    printf("[MAIN] LEDs initialized to OFF state\n");
    
    // Start LED processor on core 1
    printf("[MAIN] Launching LED processor on core 1...\n");
    multicore_launch_core1(core1_led_processor);
    
    sleep_ms(100); // Give core 1 time to start
    
    // Run DMX receiver on core 0 (this core)
    printf("[MAIN] Starting DMX receiver on core 0...\n");
    core0_dmx_receiver();
    
    // This should never be reached
    printf("[MAIN] ERROR: Main loop exited unexpectedly!\n");
    return -1;
}