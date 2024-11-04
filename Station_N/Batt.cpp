#include "Config.h"

const float PIN_VOLTAGE = 1.3;
const float MAX_BATTERY_VOLTAGE = 7.5;  // Maximum voltage of your battery
const float MIN_BATTERY_VOLTAGE = 6.0;  // Minimum voltage of your battery (adjust as needed)

// Voltage divider ratio
const float VOLTAGE_DIVIDER_RATIO = MAX_BATTERY_VOLTAGE / PIN_VOLTAGE;  // Calculated from your setup

float readBatteryVoltage() {
  int rawValue = analogRead(BATTERY_PIN);
  float voltage = rawValue * (3.3 / 1023.0);  // Convert to voltage
  // Calculate actual voltage considering the voltage divider
  float actualVoltage = voltage * VOLTAGE_DIVIDER_RATIO;
  return actualVoltage;
}

int calculateBatteryPercentage(float voltage) {
  int percentage = map(voltage * 100, MIN_BATTERY_VOLTAGE * 100, MAX_BATTERY_VOLTAGE * 100, 0, 100);
  return constrain(percentage, 0, 100);
}


void ReadBattery() {
  float batteryVoltage = readBatteryVoltage();
  batteryPercentage = calculateBatteryPercentage(batteryVoltage);
  
  // Serial.print("Battery Voltage: ");
  // Serial.println(batteryVoltage, 2);  // Print with 2 decimal places
  // Serial.print("V, Percentage: ");
  // Serial.print(batteryPercentage);
  // Serial.println("%");

  delay(500);
}