#pragma once

// ===========================================
// PicoLED Protocol Bridge Configuration
// ===========================================

// DMX512 Configuration
#define DMX_UNIVERSE_SIZE           512     // Exactly 512 channels as required
#define DMX_START_CODE              0x00    // Standard DMX512 start code
#define DMX_BREAK_TIME_US           100     // Break time in microseconds
#define DMX_MARK_TIME_US            12      // Mark after break time in microseconds

// WS2812 LED Configuration
#define DEFAULT_LED_COUNT           256     // Default number of LEDs
#define DEFAULT_GRID_WIDTH          16      // Default grid width
#define DEFAULT_GRID_HEIGHT         16      // Default grid height
#define WS2812_PIO                  pio0    // Default PIO instance
#define WS2812_SM                   0       // Default state machine

// RS485 Serial Configuration
#define RS485_DEFAULT_BAUD          115200  // Default baud rate
#define RS485_MAX_FRAME_SIZE        1024    // Maximum frame size for RS485
#define RS485_UART_INSTANCE         uart1   // Default UART instance
#define RS485_TX_TIMEOUT_MS         100     // Transmission timeout

// Pin Defaults (can be overridden in constructor)
#define DEFAULT_LED_PIN             2       // Default WS2812 data pin
#define DEFAULT_DMX_PIN             4       // Default DMX output pin
#define DEFAULT_RS485_DATA_PIN      8       // Default RS485 data pin
#define DEFAULT_RS485_ENABLE_PIN    9       // Default RS485 enable pin

// Timing Configuration
#define UPDATE_INTERVAL_MS          16      // ~60 FPS update rate
#define DMX_REFRESH_RATE_HZ         44      // Standard DMX refresh rate (max 44 Hz)

// Color conversion macros
#define RGB_TO_GRB(r, g, b)         (((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b))
#define URGB_U32(r, g, b)           (((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b))
#define URGBW_U32(r, g, b, w)       (((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | ((uint32_t)(w) << 24) | (uint32_t)(b))

// Protocol enable flags
#define ENABLE_WS2812_PROTOCOL      1       // Enable WS2812 LED support
#define ENABLE_DMX512_PROTOCOL      1       // Enable DMX512 output
#define ENABLE_RS485_PROTOCOL       1       // Enable RS485 serial

// Memory management
#define LED_BUFFER_ALIGNMENT        4       // 32-bit alignment for LED buffer
#define DMX_BUFFER_ALIGNMENT        1       // Byte alignment for DMX buffer

// Debug configuration
#define DEBUG_ENABLED               1       // Enable debug output
#define DEBUG_UART                  uart0   // Debug output UART
#define DEBUG_BAUD_RATE            115200   // Debug UART baud rate

// Performance optimization
#define USE_DMA_FOR_LED_UPDATE      1       // Use DMA for LED updates if available
#define USE_MULTICORE              0       // Enable multicore processing (experimental)

// Safety limits
#define MAX_LED_COUNT               1024    // Maximum number of LEDs supported
#define MAX_BRIGHTNESS              255     // Maximum brightness value
#define MIN_UPDATE_INTERVAL_MS      1       // Minimum update interval

// Protocol-specific timing
#define WS2812_RESET_TIME_US        280     // WS2812 reset time
#define DMX512_INTER_SLOT_TIME_US   4       // Inter-slot time for DMX512
#define RS485_TURNAROUND_TIME_US    50      // RS485 direction switching time