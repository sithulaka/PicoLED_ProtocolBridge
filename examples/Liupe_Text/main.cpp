#include <stdio.h>
#include <stdlib.h>
#include "pico/multicore.h"
#include "DmxInput.h"
#include "led.h"


#define IS_RGBW false
#define NUM_PIXELS 64
#define WS2812_PIN 16
#define WS2812_FREQ 800000

PIO pio;
uint sm;
uint offset;

LED led(pio, sm, NUM_PIXELS);

void core1_entry() {
    while (1) {
        led.fast_set_XY(8, 5, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(8, 6, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(8, 7, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(6, 5, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(6, 6, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(6, 7, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(7, 6, 0, 0, 100); sleep_ms(500);

        led.fast_set_XY(2, 4, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(3, 4, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(4, 4, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(4, 5, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(4, 6, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(3, 6, 0, 0, 100); sleep_ms(500);
        led.fast_set_XY(3, 5, 0, 0, 100); sleep_ms(5500);
    }
}

int main() {

    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);
 
    ws2812_program_init(pio, sm, offset, WS2812_PIN, WS2812_FREQ, IS_RGBW);
    
    led = LED(pio, sm, NUM_PIXELS);

    led.reset_all_color();
    led.Show_XY_Lines();
//  multicore_launch_core1(core1_entry);

    while (1) {
            led.change_all_color(255,255,255); led.push_array(); sleep_ms(5000);
            led.reset_all_color();
            led.fast_set_XY(8, 1, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(7, 1, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(6, 1, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(6, 2, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(6, 3, 0, 0, 100); sleep_ms(500);

            led.fast_set_XY(8, 5, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(8, 6, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(8, 7, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(6, 5, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(6, 6, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(6, 7, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(7, 6, 0, 0, 100); sleep_ms(500);

            led.fast_set_XY(4, 1, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(3, 1, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(2, 1, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(2, 2, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(2, 3, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(3, 3, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(4, 3, 0, 0, 100); sleep_ms(500);

            led.fast_set_XY(2, 4, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(3, 4, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(4, 4, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(4, 5, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(4, 6, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(3, 6, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(3, 5, 0, 0, 100); sleep_ms(500);

            led.fast_set_XY(4, 8, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(4, 7, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(3, 7, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(2, 7, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(2, 8, 0, 0, 100); sleep_ms(500);
            led.fast_set_XY(3, 8, 0, 0, 100); sleep_ms(2000);

            led.change_all_avalible_color (100, 0, 0); 
            led.push_array(); sleep_ms(1000);
            led.change_all_avalible_color(0, 100, 0); 
            led.push_array(); sleep_ms(1000);
            led.change_all_avalible_color (100, 100, 100); 
            led.push_array(); sleep_ms(1000);
        };
    
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
