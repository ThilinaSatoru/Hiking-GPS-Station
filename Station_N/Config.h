#ifndef CONFIG_H
#define CONFIG_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <Arduino.h>

// ==============================================
// Display Configuration
// ==============================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

extern Adafruit_SSD1306 display;

// New global variables
extern unsigned long emergencyStartTime;
extern unsigned long emergencyDuration;

enum EmergencyDisplayState
{
  EMERGENCY_DISPLAY_IDLE,
  EMERGENCY_DISPLAY_SENDING,
  EMERGENCY_DISPLAY_SENT
};
extern EmergencyDisplayState emergencyDisplayState;
extern unsigned long emergencyDisplayTimer;
extern const unsigned long DISPLAY_SENT_DURATION;

// ==============================================
// GPS Configuration
// ==============================================
#define GPS_RX_PIN D3 // Connect to GPS TX
#define GPS_TX_PIN D4 // Connect to GPS RX
#define GPS_BAUD 9600

extern TinyGPSPlus gps;
extern SoftwareSerial gpsSerial;

// ==============================================
// Button and LED Configuration
// ==============================================
#define BUTTON1_PIN D5
#define LED1_PIN D7

// System timing constants (in milliseconds)
const unsigned long LED_TIMEOUT_MS = 2000;
const unsigned long MESH_UPDATE_INTERVAL_MS = 5000;
const unsigned long DEBUG_INTERVAL_MS = 5000;

// State variables
enum ButtonMode
{
  STATE_NORMAL = 0,
  STATE_MEDICAL = 1,
  STATE_ENVIRONMENT = 2,
  STATE_RESOURCES = 3
};
extern ButtonMode currentButtonMode;
extern ButtonMode selectedButtonMode;
extern bool led1State;
extern bool lastButton1State;

// ==============================================
// Mesh Network Configuration
// ==============================================
#define MESH_PREFIX "HIKING"
#define MESH_PASSWORD "meshPassword"
#define MESH_PORT 5555
#define STATION_NUMBER 1

extern painlessMesh mesh;
extern Scheduler meshScheduler;
extern uint32_t nodeId;

// ==============================================
// GPS Configuration
// ==============================================
#if STATION_NUMBER == 1
#define DEFAULT_LAT 7.399366
#define DEFAULT_LNG 80.809972
#elif STATION_NUMBER == 2
#define DEFAULT_LAT 7.400771
#define DEFAULT_LNG 80.810616
#endif

// ==============================================
// Battery Configuration
// ==============================================
#define BATTERY_PIN A0
extern int batteryPercentage;

// =====================================================================
// Function Declarations
// =====================================================================

// Display functions
void setupDisplay();
void updateDisplay();
void displayGPSData();
void displayMeshStatus();
void displayEmergencyStatus();

// GPS functions
void setupGPS();
void updateGPS();
bool isGPSValid();
float getLatitude();
float getLongitude();
void displayGPSInfo();

// Button and LED functions
void setupButtons();
void updateLEDForState();
void resetButtonCycle();
void handleButtons();

// Mesh functions
void setupMesh();
void sendMeshData();
String getMeshData();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);

void ReadBattery();
// =====================================================================

#endif