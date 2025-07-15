/*
 * Copyright (c) 2021 Jostein Løwer 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Description: 
 * DMX Universe Receiver - Receives DMX data and converts to GRB serial output
 * for LED display panels using PicoLED class
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "DmxInput.h"
#include "PicoLED.h"
#include "config.h"

DmxInput dmxInput;
PicoLED* led;

#define LED_PIN PICO_DEFAULT_LED_PIN

volatile uint8_t buffer[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];

void setup_led_output() {
    // Initialize PicoLED for GRB serial output to display panel
    led = new PicoLED(pio0, 0, NUM_PIXELS, GRID_WIDTH);
    printf("PicoLED initialized: %d pixels, %dx%d grid, WS2812 pin %d\n", 
           NUM_PIXELS, GRID_WIDTH, GRID_HEIGHT, WS2812_PIN);
}

void process_dmx_to_grb() {
    // Convert DMX buffer to LED array using PicoLED converter
    led->DmxArray_to_GRBArray_Converter((const uint8_t*)buffer);
    
    // Push the converted data to the WS2812 LEDs (GRB serial output)
    led->push_array();
    
    printf("DMX data converted and output to GRB display panel\n");
}

int main()
{
    stdio_init_all();
    printf("DMX Universe Receiver with GRB Serial Output starting...\n");
    
    // Setup DMX Input on GPIO 0 (same as original working code)
    dmxInput.begin(0, START_CHANNEL, NUM_CHANNELS);
    printf("DMX Input initialized on GPIO %d, channels %d-%d\n", 
           0, START_CHANNEL, START_CHANNEL + NUM_CHANNELS - 1);

    // Initialize PicoLED for GRB output
    setup_led_output();

    // Setup the onboard LED for status indication
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    printf("Ready to receive DMX universes and output to GRB display panel\n");
    
    while (true)
    {
        printf("[DEBUG] Waiting for DMX packet...\n");
        
        // Wait for next DMX packet
        dmxInput.read(buffer);
        
        printf("[DEBUG] DMX packet received! Buffer size: %d bytes\n", sizeof(buffer));

        // Convert DMX data to GRB and output to serial display panel
        process_dmx_to_grb();

        // Debug: Print first few DMX channels (RGB of first LED)
        printf("[DEBUG] DMX Ch1-3: R=%d G=%d B=%d\n", buffer[0], buffer[1], buffer[2]);
        
        // Debug: Print more channel data
        printf("[DEBUG] DMX Ch4-9: %d,%d,%d,%d,%d,%d\n", 
               buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);

        // Blink status LED to indicate packet received and processed
        gpio_put(LED_PIN, 1);
        sleep_ms(10);
        gpio_put(LED_PIN, 0);
        
        printf("[DEBUG] Packet processed, LED blinked\n");
    }
    
    return 0;
}