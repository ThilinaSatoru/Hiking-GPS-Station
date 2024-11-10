#include "Config.h"

bool wasEmergencyActive = false;
bool emergencyJustSent = false;
// unsigned long emergencyStartTime = 0;
// unsigned long emergencyDuration = 0;

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
