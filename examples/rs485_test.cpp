#include "../include/PicoLED.h"
#include "pico/stdlib.h"
#include <cstdio>

/**
 * @brief RS485 Serial Communication Test
 * 
 * This example demonstrates:
 * - Sending various data types via RS485 (simplex)
 * - Variable frame lengths
 * - Status monitoring and diagnostics
 */

int main() {
    stdio_init_all();

    // Configure pins - focus on RS485 communication
    PicoLED::PinConfig pins = {
        .led_panel_pin = 2,
        .dmx512_pin = 4,
        .rs485_data_pin = 8,
        .rs485_enable_pin = 9
    };

    // Minimal LED config for this test
    PicoLED::LEDConfig led_config = {
        .num_pixels = 16,
        .grid_width = 4,
        .grid_height = 4,
        .pio_instance = pio0,
        .pio_sm = 0
    };

    PicoLED picoled(pins, led_config);

    if (!picoled.begin()) {
        printf("ERROR: Failed to initialize PicoLED!\n");
        return -1;
    }

    printf("RS485 Communication Test Started!\n");
    printf("Baud Rate: 115200\n");
    printf("Mode: Simplex (Transmit Only)\n\n");

    uint32_t test_count = 0;

    while (true) {
        test_count++;

        // Test 1: Send simple status message
        if (test_count % 100 == 1) {
            char status[64];
            snprintf(status, sizeof(status), "[STATUS] Test %lu - System Running\n", test_count);
            picoled.sendRS485String(status);
            printf("Sent status message\n");
        }

        // Test 2: Send binary data frame
        if (test_count % 150 == 1) {
            uint8_t binary_data[] = {
                0x55, 0xAA,                           // Sync bytes
                0x01,                                 // Command: Data packet
                0x08,                                 // Length
                (uint8_t)(test_count & 0xFF),         // Data bytes
                (uint8_t)((test_count >> 8) & 0xFF),
                (uint8_t)((test_count >> 16) & 0xFF),
                (uint8_t)((test_count >> 24) & 0xFF),
                0x00, 0x00, 0x00, 0x00,              // Padding
                0x00  // Checksum (placeholder)
            };
            
            // Calculate simple checksum
            uint8_t checksum = 0;
            for (int i = 0; i < sizeof(binary_data) - 1; i++) {
                checksum ^= binary_data[i];
            }
            binary_data[sizeof(binary_data) - 1] = checksum;

            picoled.sendRS485Frame(binary_data, sizeof(binary_data));
            printf("Sent binary frame (%d bytes)\n", (int)sizeof(binary_data));
        }

        // Test 3: Send sensor data simulation
        if (test_count % 200 == 1) {
            // Simulate sensor readings
            float temperature = 25.0f + 5.0f * sin(test_count * 0.01f);
            float humidity = 50.0f + 20.0f * cos(test_count * 0.015f);
            uint16_t light_level = (uint16_t)(512 + 300 * sin(test_count * 0.02f));

            char sensor_data[128];
            snprintf(sensor_data, sizeof(sensor_data), 
                    "SENSOR,%.1f,%.1f,%u\n", 
                    temperature, humidity, light_level);
            
            picoled.sendRS485String(sensor_data);
            printf("Sent sensor data: T=%.1f°C, H=%.1f%%, L=%u\n", 
                   temperature, humidity, light_level);
        }

        // Test 4: Send JSON-formatted data
        if (test_count % 300 == 1) {
            char json_data[256];
            snprintf(json_data, sizeof(json_data), 
                    "{\"id\":%lu,\"uptime\":%lu,\"protocols\":{\"dmx\":true,\"led\":true,\"rs485\":true}}\n",
                    test_count, to_ms_since_boot(get_absolute_time()));
            
            picoled.sendRS485String(json_data);
            printf("Sent JSON data\n");
        }

        // Test 5: Send large data packet
        if (test_count % 500 == 1) {
            uint8_t large_packet[256];
            large_packet[0] = 0xFF;  // Start marker
            large_packet[1] = 0xFE;  // Packet type
            large_packet[2] = 252;   // Data length (256 - 4 header bytes)
            
            // Fill with pattern data
            for (int i = 3; i < 255; i++) {
                large_packet[i] = (uint8_t)(i ^ (test_count & 0xFF));
            }
            
            large_packet[255] = 0xDD;  // End marker
            
            picoled.sendRS485Frame(large_packet, sizeof(large_packet));
            printf("Sent large packet (256 bytes)\n");
        }

        // Test 6: Variable length messages
        if (test_count % 75 == 1) {
            uint8_t msg_len = 10 + (test_count % 50);  // Variable length 10-59 bytes
            uint8_t var_msg[64];
            
            var_msg[0] = 0xA5;  // Header
            var_msg[1] = msg_len - 2;  // Payload length
            
            // Fill with incremental pattern
            for (uint8_t i = 2; i < msg_len; i++) {
                var_msg[i] = i + (test_count & 0xFF);
            }
            
            picoled.sendRS485Frame(var_msg, msg_len);
            printf("Sent variable message (%u bytes)\n", msg_len);
        }

        // Status check and diagnostics
        if (test_count % 1000 == 0) {
            printf("\n=== RS485 Diagnostics (Test %lu) ===\n", test_count);
            printf("RS485 Busy: %s\n", picoled.isRS485Busy() ? "Yes" : "No");
            
            // Print general status
            picoled.printStatus();
            printf("==============================\n\n");
        }

        // Baud rate change test
        if (test_count == 2000) {
            printf("Changing baud rate to 57600...\n");
            picoled.sendRS485String("BAUD_CHANGE:57600\n");
            sleep_ms(100);
            picoled.setRS485BaudRate(57600);
        } else if (test_count == 4000) {
            printf("Changing baud rate back to 115200...\n");
            picoled.sendRS485String("BAUD_CHANGE:115200\n");
            sleep_ms(100);
            picoled.setRS485BaudRate(115200);
        }

        // Update LED panel with simple pattern to show activity
        uint8_t brightness = (uint8_t)(64 + 32 * sin(test_count * 0.1));
        picoled.setAllLEDs(brightness, 0, 0);
        picoled.updateLEDPanel();

        sleep_ms(50);  // 20 Hz update rate
    }

    return 0;
}