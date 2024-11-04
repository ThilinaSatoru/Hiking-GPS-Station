#include "Config.h"

const float PIN_VOLTAGE = 1.3;
const float MAX_BATTERY_VOLTAGE = 7.5;  
const float MIN_BATTERY_VOLTAGE = 6.0;  

const float VOLTAGE_DIVIDER_RATIO = MAX_BATTERY_VOLTAGE / PIN_VOLTAGE;

// Parameters for stable reading
static float smoothedVoltage = 0;
static const int REQUIRED_SAMPLES = 10;     // Number of consistent readings needed before changing
static const float VOLTAGE_THRESHOLD = 0.1; // Minimum voltage change to consider updating
static int consistentReadings = 0;
static bool isInitialized = false;

float readBatteryVoltage() {
  int rawValue = analogRead(BATTERY_PIN);
  float voltage = rawValue * (3.3 / 1023.0);  
  float actualVoltage = voltage * VOLTAGE_DIVIDER_RATIO;
  return actualVoltage;
}

int calculateBatteryPercentage(float voltage) {
  int percentage = map(voltage * 100, MIN_BATTERY_VOLTAGE * 100, MAX_BATTERY_VOLTAGE * 100, 0, 100);
  return constrain(percentage, 0, 100);
}

void ReadBattery() {
  float currentVoltage = readBatteryVoltage();
  
  // Initialize on first reading
  if (!isInitialized) {
    smoothedVoltage = currentVoltage;
    batteryPercentage = calculateBatteryPercentage(smoothedVoltage);
    isInitialized = true;
    return;
  }
  
  // Check if current reading is significantly different
  if (abs(currentVoltage - smoothedVoltage) > VOLTAGE_THRESHOLD) {
    // If voltage is consistently different, increment counter
    if (currentVoltage < smoothedVoltage) {
      consistentReadings++;
    } else {
      consistentReadings = 0;  // Reset if inconsistent
    }
    
    // Only update if we have enough consistent readings
    if (consistentReadings >= REQUIRED_SAMPLES) {
      // Gradually update the smoothed voltage (mobile phone style)
      smoothedVoltage = smoothedVoltage * 0.95 + currentVoltage * 0.05;
      
      // Calculate new percentage
      int newPercentage = calculateBatteryPercentage(smoothedVoltage);
      
      // Only update if percentage actually changed
      if (newPercentage != batteryPercentage) {
        batteryPercentage = newPercentage;
        consistentReadings = 0;  // Reset counter
      }
    }
  } else {
    consistentReadings = 0;  // Reset counter if voltage is stable
  }
  
  delay(500);
}