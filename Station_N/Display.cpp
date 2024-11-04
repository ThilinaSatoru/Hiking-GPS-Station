#include "Config.h"

bool wasEmergencyActive = false;
bool emergencyJustSent = false;

void setupDisplay() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Initializing...");
    display.display();
}

void centerText(const char* text, int y, int textSize) {
  display.setTextSize(textSize);  // Set the text size
  int16_t x1, y1;
  uint16_t width, height;
  // Get the width of the text in pixels
  display.getTextBounds(text, 0, y, &x1, &y1, &width, &height);
  // Calculate the x position to center the text
  int x = (display.width() - width) / 2;
  // Set the cursor to the calculated position
  display.setCursor(x, y);
  // Print the text
  display.println(text);
}

unsigned long getEmergencyDurationMinutes() {
    if (EMERGENCY) {
        return (millis() - emergencyStartTime) / 60000; // Convert milliseconds to minutes
    }
    return emergencyDuration / 60000;
}

void updateEmergencyStatus() {
    if (EMERGENCY && !wasEmergencyActive) {
        // Emergency just became active
        emergencyDisplayState = EMERGENCY_DISPLAY_SENDING;
        // emergencyStartTime = millis();
        wasEmergencyActive = true;
    } else if (!EMERGENCY && wasEmergencyActive) {
        // Emergency just ended
        emergencyDisplayState = EMERGENCY_DISPLAY_SENT;
        emergencyDisplayTimer = millis();
        // emergencyDuration = millis() - emergencyStartTime;
        wasEmergencyActive = false;
        emergencyJustSent = true;
    } else if (!EMERGENCY && emergencyJustSent && 
               (millis() - emergencyDisplayTimer > DISPLAY_SENT_DURATION)) {
        // After showing "SOS Sent" for DISPLAY_SENT_DURATION
        emergencyDisplayState = EMERGENCY_DISPLAY_IDLE;
        emergencyJustSent = false;
    }
}

void displayEmergencyStatus() {
    switch (emergencyDisplayState) {
        case EMERGENCY_DISPLAY_IDLE:
            display.println("Press SOS");
            break;
        case EMERGENCY_DISPLAY_SENDING:
            display.println("Sending SOS");
            break;
        case EMERGENCY_DISPLAY_SENT:
            display.println("SOS Sent to Emergency Services");
            break;
    }
}

void displayDuration() {
  char durationText[20];
  snprintf(durationText, sizeof(durationText), "SOS sent %lu min ago", getEmergencyDurationMinutes());
  display.println(durationText);
}

void displayGPSData() {
  double currentLat = gps.location.lat();
  double currentLng = gps.location.lng();

  if (gps.location.isValid()) {
    display.setTextSize(1);
    // Latitude row
    display.print("Lat: ");
    // display.setCursor(display.width() - 60, 0);
    display.print(" ");
    display.print(abs(currentLat), 4);
    display.println((currentLat >= 0) ? " N" : " S");
    // Longitude row
    display.print("Lng: ");
    // display.setCursor(display.width() - 60, 8);
    display.print(abs(currentLng), 4);
    display.println((currentLng >= 0) ? " E" : " W");

  } else {
    display.println("GPS Loading...");
  }
}

void displayMeshStatus() { 
    if(mesh.getNodeList().size() == 0) {
      display.println("Connecting...");
    } else {
      display.print("Connected Stations: ");display.print(mesh.getNodeList().size());
    }
}

void displayBatteryStatus() {
    display.print("Battery: ");display.print(batteryPercentage);display.println("%");
}

void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0,0);
    updateEmergencyStatus();
    // Display all information in sections
    displayEmergencyStatus();
    display.println();
    displayDuration();
    displayGPSData();
    displayMeshStatus();
    displayBatteryStatus();
    
    display.display();
}