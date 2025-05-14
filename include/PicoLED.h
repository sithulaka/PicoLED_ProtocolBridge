#pragma once

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "config.h"

class PicoLED{
    PIO pio;
    uint sm;
    uint num_pixels;
    uint32_t pixel_grb;
    uint32_t *led_array;
    uint grid_width;  // Width of the LED grid

    void put_pixel(uint32_t pixel_grb) {
        pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
    }

    static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)(r) << 8) |
               ((uint32_t)(g) << 16) |
               (uint32_t)(b);
    }

    static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
        return ((uint32_t)(r) << 8) |
               ((uint32_t)(g) << 16) |
               ((uint32_t)(w) << 24) |
               (uint32_t)(b);
    }

public:
    PicoLED(PIO pio, uint sm, uint num_pixels, uint grid_width = GRID_WIDTH);

    void fast_set_color(uint address, uint8_t r, uint8_t g, uint8_t b);

    void set_color(uint address, uint8_t r, uint8_t g, uint8_t b);

    void change_all_color(uint8_t r, uint8_t g, uint8_t b);

    void change_all_avalible_color(uint8_t r, uint8_t g, uint8_t b);

    void reset_all_color();

    void iterate_led(uint8_t r, uint8_t g, uint8_t b, uint t);

    void Show_XY_Lines();

    void fast_set_XY(uint X, uint Y, uint8_t r, uint8_t g, uint8_t b);

    void set_XY(uint X, uint Y, uint8_t r, uint8_t g, uint8_t b);

    void DmxArray_to_GRBArray_Converter(const uint8_t* DmxArray);

    void push_array();

    ~PicoLED();
};
