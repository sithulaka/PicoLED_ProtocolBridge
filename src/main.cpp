#include <stdio.h>
#include <stdlib.h>
#include "pico/multicore.h"
#include "DmxInput.h"
#include "PicoLED.h"
#include "config.h"

PIO pio;
uint sm;
uint offset;

DmxInput dmxInput;
PicoLED led(pio, sm, NUM_PIXELS, GRID_WIDTH);

// uint8_t *dmxArray;
volatile uint8_t buffer[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];

void __isr dmxDataRecevied(DmxInput* instance) {
    uint32_t data = 1;
    multicore_fifo_push_blocking(data);
  }

void Main_Core0() {
    DmxInput::return_code result = dmxInput.begin(DMX_IN_PIN, START_CHANNEL, NUM_CHANNELS);
    if (result != DmxInput::SUCCESS) {
        // Handle the error - blink LED or similar
        while (1) {
            // Flash red LED as error indicator
            led.fast_set_color(1, 255, 0, 0);
            sleep_ms(200);
            led.fast_set_color(1, 0, 0, 0);
            sleep_ms(200);
        }
    }
    dmxInput.read_async(buffer, dmxDataRecevied);
}

void Main_Core1() {
    while (1) {
        uint32_t recived_data = multicore_fifo_pop_blocking();
        if (recived_data == 1) {
            recived_data = 0;
            
            // Create a non-volatile copy
            uint8_t buffer_copy[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];
            for (int i = 0; i < DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS); i++) {
                buffer_copy[i] = buffer[i];
            }
            
            led.DmxArray_to_GRBArray_Converter(buffer_copy);
            sleep_ms(10);
            led.push_array();
        }
    }
}

int main() {
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    // Initialize the WS2812 PIO program
    ws2812_program_init(pio, sm, offset, WS2812_PIN, WS2812_FREQ, IS_RGBW);
    
    // Set initial color to confirm operation
    led.fast_set_color(3, 255, 0, 0);
    
    // Launch multicore operation
    multicore_launch_core1(Main_Core1);
    Main_Core0();

    // Cleanup
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
