#include "PicoLED.h"

PicoLED::PicoLED(PIO pio, uint sm, uint num_pixels) {
    this->pio = pio;
    this->sm = sm;
    this->num_pixels = num_pixels;
    led_array = new uint32_t[num_pixels];
}

void PicoLED::fast_set_color(uint address, uint8_t r, uint8_t g, uint8_t b) {
    if (address >= num_pixels) {
        return;  // Out of bounds
    }
    pixel_grb = urgb_u32(r, g, b);
    led_array[address-1] = pixel_grb;
    push_array();
}

// This function sets the color of a specific PicoLED
void PicoLED::set_color(uint address, uint8_t r, uint8_t g, uint8_t b) {
    if (address >= num_pixels) {
        return;  // Out of bounds
    }
    pixel_grb = urgb_u32(r, g, b);

   led_array[address-1] = pixel_grb;
}


// This function sets colors for all PicoLEDs
void PicoLED::change_all_color(uint8_t r, uint8_t g, uint8_t b) {
    pixel_grb = urgb_u32(r, g, b);
    for (uint i = 0; i < num_pixels; ++i) {
       led_array[i] = pixel_grb;
    }
}


// This function sets colors for all available PicoLEDs
void PicoLED::change_all_avalible_color(uint8_t r, uint8_t g, uint8_t b) {
    pixel_grb = urgb_u32(r, g, b);
    for (uint i = 0; i < num_pixels; ++i) {
        if (led_array[i] != 0) {
           led_array[i] = pixel_grb;
        }
    }
}

void PicoLED::DmxArray_to_GRBArray_Converter(const uint8_t* DmxArray){
    uint index = 0;
    for (uint i = 0; i < 193; i+=3) {
        pixel_grb = urgb_u32(DmxArray[i], DmxArray[i+1], DmxArray[i+2]);

        if (index < num_pixels) {
            led_array[index] = pixel_grb;
        }

        index++;
    }

}

// Function to reset all PicoLEDs to off state
void PicoLED::reset_all_color() {
    change_all_color(0, 0, 0);
}

void PicoLED::push_array() {
    for (uint i = 0; i < num_pixels; ++i) {
        put_pixel(led_array[i]);
    }
}


void PicoLED::iterate_led(uint8_t r, uint8_t g, uint8_t b, uint t) {
    pixel_grb = urgb_u32(r, g, b);
    for (uint i = 0; i < num_pixels; ++i) {
        if (i <= t) {
            put_pixel(pixel_grb);
        } else {
            put_pixel(0);
        }
    }
}

void PicoLED::Show_XY_Lines(){
    for (uint i = 0; i < 8; ++i) {
        fast_set_color(i, 255, 0, 0);
        sleep_ms(100);
        }
    for (uint i = 0; i < 60; i+=8) {
        fast_set_color(i, 255, 0, 0);
        sleep_ms(100);
        }
    sleep_ms(1500);
    reset_all_color(); push_array();
}



void PicoLED::fast_set_XY(uint X, uint Y, uint8_t r, uint8_t g, uint8_t b) {
    uint address = (X + ((Y-1) * 8));  // Assuming a grid of 8x8 PicoLEDs
    if (address >= num_pixels) {
        return;  // Out of bounds
    }
    pixel_grb = urgb_u32(r, g, b);
   led_array[address-1] = pixel_grb;
    push_array();
}


void PicoLED::set_XY(uint X, uint Y, uint8_t r, uint8_t g, uint8_t b) {
    uint address = (X + ((Y-1) * 8));
    if (address >= num_pixels) {
        return;  // Out of bounds
    }
    pixel_grb = urgb_u32(r, g, b);

   led_array[address-1] = pixel_grb;
}


PicoLED::~PicoLED() {
    delete[]led_array;
}
