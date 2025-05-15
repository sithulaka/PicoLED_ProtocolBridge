/*
 * DMX Test Pattern Generator for Arduino Uno
 * 
 * This sketch sends various DMX test patterns using the DMXSerial library
 * for testing DMX receivers and fixtures. It requires a DMX shield with MAX485 chip.
 * 
 * Hardware Requirements:
 * - Arduino Uno
 * - DMX Shield with MAX485 chip (connected to pins 0-TX, 1-RX, and 2-DE/RE)
 */

#include <DMXSerial.h>

// Define pins for the DMX shield if needed
// Pin 2 is often used for DE/RE control on many DMX shields
#define DMX_DIR_PIN 2  // Direction control pin (if your shield uses it)

// Define the number of DMX channels to use
#define DMX_CHANNELS 32

// Test pattern selector
uint8_t currentPattern = 0;
uint32_t lastPatternChange = 0;
uint32_t patternDuration = 5000; // Change pattern every 5 seconds

// Variables for pattern animation
uint8_t fadeValue = 0;
bool fadeDirection = true;
uint8_t chasePosition = 0;
uint32_t lastUpdate = 0;
uint8_t updateDelay = 30; // 30ms update rate

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("DMX Test Pattern Generator");
  
  // Initialize the DMX library in Controller mode (sending data)
  DMXSerial.init(DMXController);
  
  // Set all DMX channels to 0
  clearAllChannels();
  
  // Print instructions
  Serial.println("Generating DMX test patterns");
  Serial.println("Patterns change every 5 seconds");
}

void loop() {
  uint32_t currentTime = millis();
  
  // Update the pattern every updateDelay milliseconds
  if (currentTime - lastUpdate >= updateDelay) {
    lastUpdate = currentTime;
    updatePattern();
  }
  
  // Change pattern every few seconds
  if (currentTime - lastPatternChange > patternDuration) {
    currentPattern = (currentPattern + 1) % 5; // Cycle through 5 patterns
    lastPatternChange = currentTime;
    
    Serial.print("Changing to pattern: ");
    Serial.println(currentPattern);
  }
}

void clearAllChannels() {
  // Set all channels to 0
  for (int i = 1; i <= DMX_CHANNELS; i++) {
    DMXSerial.write(i, 0);
  }
}

void setAllChannels(uint8_t value) {
  // Set all channels to the same value
  for (int i = 1; i <= DMX_CHANNELS; i++) {
    DMXSerial.write(i, value);
  }
}

void setChasePattern(uint8_t position, uint8_t width, uint8_t value) {
  // Reset all channels first
  clearAllChannels();
  
  // Set the channels in the chase pattern
  for (int i = 0; i < width; i++) {
    int channel = ((position + i) % DMX_CHANNELS) + 1;
    DMXSerial.write(channel, value);
  }
}

void setAlternatingPattern(uint8_t evenValue, uint8_t oddValue) {
  for (int i = 1; i <= DMX_CHANNELS; i++) {
    DMXSerial.write(i, (i % 2 == 0) ? evenValue : oddValue);
  }
}

void setRgbTestPattern(uint8_t intensity) {
  // Assuming RGB fixtures are arranged with 3 channels per fixture (R,G,B)
  clearAllChannels();
  
  for (int i = 1; i <= DMX_CHANNELS; i += 3) {
    if (i + 2 <= DMX_CHANNELS) {
      // Set Red
      DMXSerial.write(i, intensity);
      // Set Green
      DMXSerial.write(i + 1, 0);
      // Set Blue
      DMXSerial.write(i + 2, 0);
    }
  }
}

void updatePattern() {
  switch (currentPattern) {
    case 0: // Full intensity
      setAllChannels(255);
      break;
      
    case 1: // Fade in/out all channels
      setAllChannels(fadeValue);
      
      // Update fade value
      if (fadeDirection) {
        fadeValue++;
        if (fadeValue == 255) fadeDirection = false;
      } else {
        fadeValue--;
        if (fadeValue == 0) fadeDirection = true;
      }
      break;
      
    case 2: // Chase pattern
      setChasePattern(chasePosition, 5, 255);
      chasePosition = (chasePosition + 1) % DMX_CHANNELS;
      break;
      
    case 3: // Alternating channels
      setAlternatingPattern(255, 0);
      break;
      
    case 4: // RGB test (Red only)
      setRgbTestPattern(255);
      break;
  }
} 