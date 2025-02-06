#include "Config.h"

TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

// Define other global variables here
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long emergencyStartTime = 0;
unsigned long emergencyDuration = 0;
EmergencyDisplayState emergencyDisplayState = EMERGENCY_DISPLAY_IDLE;
unsigned long emergencyDisplayTimer = 0;
const unsigned long DISPLAY_SENT_DURATION = 5000; // 5 seconds

ButtonMode currentButtonMode = STATE_NORMAL;
ButtonMode selectedButtonMode = STATE_NORMAL;
bool led1State = false;
bool lastButton1State = HIGH;

painlessMesh mesh;
Scheduler meshScheduler;
uint32_t nodeId;

int batteryPercentage = 0;