#include "PicoLED.h"
#include "config.h"

PicoLED::PicoLED(PIO pio, uint sm, uint num_pixels, uint grid_width) {
    this->pio = pio;
    this->sm = sm;
    this->num_pixels = num_pixels;
    this->grid_width = grid_width;
    led_array = new uint32_t[num_pixels];
    // Initialize all LEDs to off state
    for (uint i = 0; i < num_pixels; i++) {
        led_array[i] = 0;
    }
}

void PicoLED::fast_set_color(uint address, uint8_t r, uint8_t g, uint8_t b) {
    // Adjust the address to be zero-based for checking
    if (address < 1 || address > num_pixels) {
        return;  // Out of bounds
    }
    pixel_grb = urgb_u32(r, g, b);
    led_array[address-1] = pixel_grb;
    push_array();
}

// This function sets the color of a specific PicoLED
void PicoLED::set_color(uint address, uint8_t r, uint8_t g, uint8_t b) {
    // Adjust the address to be zero-based for checking
    if (address < 1 || address > num_pixels) {
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
    // Calculate max iterations based on NUM_CHANNELS
    // Each LED uses 3 channels (R,G,B)
    uint max_iterations = (NUM_CHANNELS / 3) * 3;
    
    for (uint i = 0; i < max_iterations; i+=3) {
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
    // Show horizontal lines
    for (uint i = 1; i <= grid_width; ++i) {
        fast_set_color(i, 255, 0, 0);
        sleep_ms(100);
    }
    
    // Show vertical lines
    for (uint i = 1; i <= num_pixels; i += grid_width) {
        fast_set_color(i, 255, 0, 0);
        sleep_ms(100);
    }
    
    sleep_ms(1500);
    reset_all_color(); 
    push_array();
}

void PicoLED::fast_set_XY(uint X, uint Y, uint8_t r, uint8_t g, uint8_t b) {
    uint address = (X + ((Y-1) * grid_width));
    if (address < 1 || address > num_pixels) {
        return;  // Out of bounds
    }
    pixel_grb = urgb_u32(r, g, b);
    led_array[address-1] = pixel_grb;
    push_array();
}

void PicoLED::set_XY(uint X, uint Y, uint8_t r, uint8_t g, uint8_t b) {
    uint address = (X + ((Y-1) * grid_width));
    if (address < 1 || address > num_pixels) {
        return;  // Out of bounds
    }
    pixel_grb = urgb_u32(r, g, b);
    led_array[address-1] = pixel_grb;
}

PicoLED::~PicoLED() {
    delete[] led_array;
}
