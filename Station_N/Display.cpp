// #include "DisplayConfig.h"
#include "Config.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

void displayGPSData() {
    display.setTextSize(1);
    if (gps.location.isValid()) {
        display.print("Lat: ");
        display.println(gps.location.lat(), 6);
        display.print("Lng: ");
        display.println(gps.location.lng(), 6);
    } else {
        display.println("GPS: No Fix");
    }
}

void displayMeshStatus() {
    display.print("Node: ");
    display.println(mesh.getNodeId());
    display.print("Connections: ");
    display.println(mesh.getNodeList().size());
}

void displayEmergencyStatus() {
    display.print("Emergency: ");
    display.println(EMERGENCY ? "YES" : "NO");
    display.print("Help: ");
    display.println(HELP ? "YES" : "NO");
    // display.print("LED1: ");
    // display.println(led1State ? "ON" : "OFF");
    // display.print("LED2: ");
    // display.println(led2State ? "ON" : "OFF");
}

void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0,0);
    
    // Display all information in sections
    displayGPSData();
    display.println(); // Add a blank line between sections
    displayMeshStatus();
    display.println();
    displayEmergencyStatus();
    
    display.display();
}