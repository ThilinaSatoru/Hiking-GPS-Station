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

unsigned long getEmergencyDurationSeconds() {
    if (emergencyDisplayState == EMERGENCY_DISPLAY_SENDING) {
        return (millis() - emergencyStartTime) / 1000; // Convert milliseconds to seconds
    }
    return emergencyDuration / 1000; // Return the final duration in seconds
}

void updateEmergencyStatus() {
    if (EMERGENCY && !wasEmergencyActive) {
        // Emergency just became active
        emergencyDisplayState = EMERGENCY_DISPLAY_SENDING;
        emergencyStartTime = millis(); // Start timing when entering SENDING state
        wasEmergencyActive = true;
    } else if (!EMERGENCY && wasEmergencyActive) {
        // Emergency just ended
        emergencyDisplayState = EMERGENCY_DISPLAY_SENT;
        emergencyDisplayTimer = millis();
        emergencyDuration = millis() - emergencyStartTime; // Store final duration
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
            display.println("Press SOS Button");
            display.println("");
            break;
        case EMERGENCY_DISPLAY_SENDING:
            display.println("Sending SOS");
            char sendingText[20];
            snprintf(sendingText, sizeof(sendingText), "Time elapsed: %lu sec", getEmergencyDurationSeconds());
            display.println(sendingText);
            display.println("");
            break;
        case EMERGENCY_DISPLAY_SENT:
            display.println("SOS Sent to Emergency Services");
            display.println("");
            char durationText[20];
            snprintf(durationText, sizeof(durationText), "Total time: %lu sec", getEmergencyDurationSeconds());
            display.println(durationText);
            break;
    }
}

void displayGPSData() {
  double currentLat = gps.location.lat();
  double currentLng = gps.location.lng();

  if (gps.location.isValid()) {
    display.setTextSize(1);
    // Latitude row
    display.print("Lat: ");
    display.print(" ");
    display.print(abs(currentLat), 4);
    display.println((currentLat >= 0) ? " N" : " S");
    // Longitude row
    display.print("Lng: ");
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
      
      display.print("Station: ");
      display.print(STATION_NUMBER);
      display.print(" (");
      display.print(mesh.getNodeList().size());
      display.println(") online");
    }
}

void displayBatteryStatus() {
    display.print("Battery: ");display.print(batteryPercentage);display.println("%");
}



void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0,0);
    updateEmergencyStatus();
    
    displayEmergencyStatus();
    displayGPSData();
    displayMeshStatus();
    displayBatteryStatus();
    
    display.display();
}