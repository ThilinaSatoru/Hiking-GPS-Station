#include "Config.h"

bool wasEmergencyActive = false;
bool emergencyJustSent = false;

void setupDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Initializing...");
    display.display();
}

void updateEmergencyStatus() {
    if (selectedButtonMode == STATE_EMERGENCY && !wasEmergencyActive) {
        // Emergency just became active
        emergencyDisplayState = EMERGENCY_DISPLAY_SENDING;
        emergencyStartTime = millis(); // Start timing when entering SENDING state
        wasEmergencyActive = true;
    } else if (selectedButtonMode == STATE_NORMAL && wasEmergencyActive) {
        // Emergency just ended
        emergencyDisplayState = EMERGENCY_DISPLAY_SENT;
        emergencyDisplayTimer = millis();
        emergencyDuration = millis() - emergencyStartTime; // Store final duration
        wasEmergencyActive = false;
        emergencyJustSent = true;
    } else if (selectedButtonMode == STATE_NORMAL && emergencyJustSent &&
               (millis() - emergencyDisplayTimer > DISPLAY_SENT_DURATION)) {
        // After showing "SOS Sent" for DISPLAY_SENT_DURATION
        emergencyDisplayState = EMERGENCY_DISPLAY_IDLE;
        emergencyJustSent = false;
    }
}

void displayEmergencyStatus() {
    display.setTextSize(1);
    switch (selectedButtonMode) {
        case STATE_NORMAL:
            display.println("Press SOS Button for Help");
            break;
        case STATE_EMERGENCY:
            display.println("Emergency!!");
            display.println("Sending SOS...");
            // You can include elapsed time or other details if needed
            break;
        case STATE_SECONDARY:
            display.println("Stay Alert!");
            display.println("Sending SOS...");
            break;
        case STATE_TERTIARY:
            display.println("Monitoring.");
            display.println("Sending SOS...");
            break;
    }
    display.println("");
}

void displayGPSData() {
    double currentLat = gps.location.lat();
    double currentLng = gps.location.lng();

    if (gps.location.isValid()) {
        display.setTextSize(1);
        display.print("Lat: ");
        display.print(abs(currentLat), 4);
        display.println((currentLat >= 0) ? " N" : " S");

        display.print("Lng: ");
        display.print(abs(currentLng), 4);
        display.println((currentLng >= 0) ? " E" : " W");
    } else {
        display.println("GPS Loading...");
    }
}

void displayMeshStatus() {
    if (mesh.getNodeList().size() == 0) {
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
    display.print("Battery: ");
    display.print(batteryPercentage);
    display.println("%");
}

void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
    
    updateEmergencyStatus();
    displayEmergencyStatus();
    displayGPSData();
    displayMeshStatus();
    displayBatteryStatus();

    display.display();
}
