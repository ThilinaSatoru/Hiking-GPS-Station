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
