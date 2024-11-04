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

enum EmergencyDisplayState {
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
#define GPS_RX_PIN D3  // Connect to GPS TX
#define GPS_TX_PIN D4  // Connect to GPS RX
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
extern bool led1State;
extern bool lastButton1State;
extern unsigned long led2Timer;

// ==============================================
// Mesh Network Configuration
// ==============================================
#define MESH_PREFIX     "HIKING"
#define MESH_PASSWORD   "meshPassword"
#define MESH_PORT       5555
#define STATION_NUMBER  2

extern painlessMesh mesh;
extern Scheduler meshScheduler;
extern bool EMERGENCY;
extern uint32_t nodeId;

// ==============================================
// Battery Configuration
// ==============================================
#define BATTERY_PIN  A0
extern int batteryPercentage;

// ==============================================
// System Status Structure
// ==============================================
struct SystemStatus {
    bool isGPSFixed;
    bool isMeshConnected;
    bool isEmergency;
    int batteryPercentage;
    int satelliteCount;
    int meshNodeCount;
    float lastLatitude;
    float lastLongitude;
};

extern SystemStatus systemStatus;

// ==============================================
// Function Declarations
// ==============================================
// Display functions
void setupDisplay();
void updateDisplay();
void displayGPSData();
void displayMeshStatus();
void displayEmergencyStatus();
unsigned long getEmergencyDurationMinutes();

// GPS functions
void setupGPS();
void updateGPS();
bool isGPSValid();
float getLatitude();
float getLongitude();
void displayGPSInfo();

// Button and LED functions
void setupButtons();
void handleButtons();
void handleLEDStates();
void updateLEDState();

// Mesh functions
void setupMesh();
void sendMeshData();
String getMeshData();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);

// Utility functions
void updateSystemStatus();
void printDebugInfo();
void handleErrors();

void ReadBattery();

#endif