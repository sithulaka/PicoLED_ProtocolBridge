#include <stdio.h>
#include <stdlib.h>
#include "pico/multicore.h"
#include "DmxInput.h"
#include "PicoLED.h"

#define IS_RGBW false
#define START_CHANNEL 1
#define NUM_CHANNELS 255
#define NUM_PIXELS 64
#define DMX_IN_PIN 1
#define WS2812_PIN 16
#define WS2812_FREQ 800000

PIO pio;
uint sm;
uint offset;

DmxInput dmxInput;
PicoLED led(pio, sm, NUM_PIXELS);

// uint8_t *dmxArray;
volatile uint8_t buffer[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];

void __isr dmxDataRecevied(DmxInput* instance) {
    uint32_t data = 1;
    multicore_fifo_push_blocking(data);
  }

void Main_Core0() {
    dmxInput.begin(DMX_IN_PIN, START_CHANNEL, NUM_CHANNELS);
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

    led = PicoLED(pio, sm, NUM_PIXELS);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, WS2812_FREQ, IS_RGBW);
    led.fast_set_color(3, 255, 0, 0);
    multicore_launch_core1(Main_Core1);
    Main_Core0();

    // Cleanup
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);

}
