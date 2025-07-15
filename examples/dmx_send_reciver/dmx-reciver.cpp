/*
 * Copyright (c) 2021 Jostein Løwer 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Description: 
 * Starts a DMX Input on GPIO pin 0 and read channel 1-3 repeatedly
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "DmxInput.h"

DmxInput dmxInput;

#define START_CHANNEL 1
#define NUM_CHANNELS 3
#define LED_PIN PICO_DEFAULT_LED_PIN

volatile uint8_t buffer[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];

int main()
{
    stdio_init_all();
    
    // Setup our DMX Input to read on GPIO 0, from channel 1 to 3
    dmxInput.begin(0, START_CHANNEL, NUM_CHANNELS);

    // Setup the onboard LED so that we can blink when we receives packets
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    while (true)
    {
        // Wait for next DMX packet
        dmxInput.read(buffer);

        // Print the DMX channels
        printf("Received packet: ");
        for (uint i = 0; i < sizeof(buffer); i++)
        {
            printf("%d, ", buffer[i]);
        }
        printf("\n");

        // Blink the LED to indicate that a packet was received
        gpio_put(LED_PIN, 1);
        sleep_ms(10);
        gpio_put(LED_PIN, 0);
    }
    
    return 0;
}