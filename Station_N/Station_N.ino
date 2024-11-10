#include "Config.h"

void setup() {
    Serial.begin(115200);
    
    setupDisplay();
    setupGPS();
    setupButtons();
    setupMesh();
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("System Ready!");
    display.display();
    
    delay(2000);
}/

void loop() {
    mesh.update();
    handleButtons();
    // updateGPS();
    // updateDisplay();
    // ReadBattery();
    
    delay(50);
}