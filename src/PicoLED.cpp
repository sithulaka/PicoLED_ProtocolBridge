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
    // Calculate max iterations based on available pixels and channels
    // Each LED uses 3 channels (R,G,B)
    uint max_leds = NUM_CHANNELS / 3;
    if (max_leds > num_pixels) {
        max_leds = num_pixels;
    }
    
    for (uint i = 0; i < max_leds; i++) {
        // DMX data starts from index 0 (R,G,B,R,G,B,...)
        uint8_t r = DmxArray[i * 3];
        uint8_t g = DmxArray[i * 3 + 1];
        uint8_t b = DmxArray[i * 3 + 2];
        
        // Convert RGB to GRB format and store in LED array
        pixel_grb = urgb_u32(r, g, b);
        led_array[i] = pixel_grb;
    }
}

void PicoLED::GRBArray_to_DmxUniverse_Converter(uint8_t* dmx_universe, uint16_t start_channel) {
    // Set start code to 0 (standard DMX)
    dmx_universe[0] = 0;
    
    // Convert LED pixels to RGB channels
    // Each LED uses 3 DMX channels (R, G, B)
    for (uint i = 0; i < num_pixels; i++) {
        uint32_t pixel = led_array[i];
        
        // Extract RGB from the GRB format used by WS2812
        uint8_t r = (pixel >> 8) & 0xFF;   // Red
        uint8_t g = (pixel >> 16) & 0xFF;  // Green  
        uint8_t b = pixel & 0xFF;          // Blue
        
        // Map to DMX channels (1-based indexing for start_channel)
        uint dmx_base = start_channel + (i * 3);
        if (dmx_base + 2 <= 512) { // DMX universe has 512 channels max
            dmx_universe[dmx_base] = r;     // Red channel
            dmx_universe[dmx_base + 1] = g; // Green channel
            dmx_universe[dmx_base + 2] = b; // Blue channel
        }
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

void PicoLED::debug_print_led_array() {
    printf("[DEBUG] LED Array contents (%d pixels):\n", num_pixels);
    uint count = 0;
    for (uint i = 0; i < num_pixels; i++) {
        if (led_array[i] != 0) {
            uint8_t r = (led_array[i] >> 8) & 0xFF;   // Red
            uint8_t g = (led_array[i] >> 16) & 0xFF;  // Green  
            uint8_t b = led_array[i] & 0xFF;          // Blue
            printf("[DEBUG] LED[%d]: R=%d G=%d B=%d (0x%08X)\n", i, r, g, b, led_array[i]);
            count++;
            if (count >= 10) { // Limit output to prevent spam
                printf("[DEBUG] ... and %d more non-zero LEDs\n", num_pixels - i - 1);
                break;
            }
        }
    }
    if (count == 0) {
        printf("[DEBUG] All LEDs are OFF (0x00000000)\n");
    }
}

PicoLED::~PicoLED() {
    delete[] led_array;
}
