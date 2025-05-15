/*
 * DMX Test Pattern Sender
 * 
 * This sketch sends various DMX test patterns using the PicoDMX library
 * for testing DMX receivers and fixtures.
 */

#include <Arduino.h>
#include <DmxOutput.h>

// Declare an instance of the DMX Output
DmxOutput dmx;

// Create a universe that we want to send
// The universe must be maximum 512 bytes + 1 byte of start code
#define UNIVERSE_LENGTH 512
uint8_t universe[UNIVERSE_LENGTH + 1];

// Test pattern selector (0-3)
uint8_t currentPattern = 0;
uint32_t lastPatternChange = 0;
uint32_t patternDuration = 5000; // Change pattern every 5 seconds

// Variables for pattern animation
uint8_t fadeValue = 0;
bool fadeDirection = true;
uint8_t chasePosition = 0;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  Serial.println("DMX Test Pattern Generator");
  
  // Start the DMX Output on GPIO-pin 0
  dmx.begin(0);
  
  // Initialize the universe with all zeros
  // Start code in universe[0] should be 0 for standard DMX
  memset(universe, 0, UNIVERSE_LENGTH + 1);
}

void setAllChannels(uint8_t value) {
  for (int i = 1; i < UNIVERSE_LENGTH + 1; i++) {
    universe[i] = value;
  }
}

void setChasePattern(uint8_t position, uint8_t width, uint8_t value) {
  // Reset all channels first
  setAllChannels(0);
  
  // Set the channels in the chase pattern
  for (int i = 0; i < width; i++) {
    int channel = ((position + i) % UNIVERSE_LENGTH) + 1;
    universe[channel] = value;
  }
}

void setAlternatingPattern(uint8_t evenValue, uint8_t oddValue) {
  for (int i = 1; i < UNIVERSE_LENGTH + 1; i++) {
    universe[i] = (i % 2 == 0) ? evenValue : oddValue;
  }
}

void setRgbTestPattern(uint8_t intensity) {
  // Assuming RGB fixtures are arranged with 3 channels per fixture
  for (int i = 1; i <= UNIVERSE_LENGTH; i += 3) {
    if (i + 2 <= UNIVERSE_LENGTH) {
      // Set Red
      universe[i] = intensity;
      // Set Green
      universe[i + 1] = 0;
      // Set Blue
      universe[i + 2] = 0;
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
      setChasePattern(chasePosition, 10, 255);
      chasePosition = (chasePosition + 1) % UNIVERSE_LENGTH;
      break;
      
    case 3: // Alternating channels
      setAlternatingPattern(255, 0);
      break;
      
    case 4: // RGB test (Red only)
      setRgbTestPattern(255);
      break;
  }
}

void loop() {
  // Update the pattern
  updatePattern();
  
  // Send the DMX universe
  dmx.write(universe, UNIVERSE_LENGTH + 1);
  
  // Wait until transmission is complete
  while (dmx.busy()) {
    // Do nothing while DMX transmits
  }
  
  // Change pattern every few seconds
  uint32_t currentTime = millis();
  if (currentTime - lastPatternChange > patternDuration) {
    currentPattern = (currentPattern + 1) % 5; // Cycle through 5 patterns
    lastPatternChange = currentTime;
    
    Serial.print("Changing to pattern: ");
    Serial.println(currentPattern);
  }
  
  // Short delay for stability
  delay(30);
} 